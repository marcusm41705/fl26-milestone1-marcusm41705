#include "aiws/retrieval_engine.hpp"
#include "aiws/text_processor.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace aiws {

    //helper functions for search
std::vector<std::string> unique_terms(const std::string& query){
    std::vector<std::string> term_vector = TextProcessor::terms(query);
    std::vector<std::string> unique_vector;
    for(const std::string& term:term_vector){
        bool found = false;
        for(const std::string& exist :unique_vector){
            if(exist == term){
                found =  true;
                break;
            }
        }
        if(!found){
            unique_vector.push_back(term);
        }
    }
    return unique_vector;
}

struct CandidateScore{
    std::size_t chunk_index{};
    double b_score{};
    std::size_t match_terms{};
    double score{};
};

std::vector<CandidateScore> collect_candidates( const std::vector<std::string>& query_terms_vector, const CorpusIndex& index, std::size_t total_chunks){

    std::vector<CandidateScore> candidate_vector;
for(const std::string& term: query_terms_vector){
    const auto* post = index.postings(term);
    if(post == nullptr){
        continue;
    }
    std::size_t doc_frequency = post->size();
    double id_freq = std::log((static_cast<double>(total_chunks) + 1.0) / (static_cast<double>(doc_frequency) + 1.0)) + 1.0;
    for(const CorpusIndex::Posting& posting: *post){
        double term_frequency = 1.0 + std::log(static_cast<double>(posting.frequency));
        CandidateScore* candidate = nullptr; //Is this chunk already a candidate?
        for(CandidateScore& current_candid : candidate_vector){
            if(current_candid.chunk_index == posting.chunk_index){
                candidate = &current_candid;
                break;
            }
        }
        if(candidate == nullptr){ //first term found inside current chunk
            CandidateScore new_candid;
            new_candid.chunk_index = posting.chunk_index;
            candidate_vector.push_back(new_candid);
            candidate = &candidate_vector.back();
        
        }
        candidate->b_score += term_frequency * id_freq;
        ++candidate->match_terms;
    }
}
return candidate_vector;
}



double RetrievalEngine::canonical_score(double) {
    // TODO: return the score in the required canonical form.
    
    return 0.0;
}


std::vector<SearchResult> RetrievalEngine::search(const std::string& query,
                                                  int k,
                                                  const std::vector<Chunk>& chunk_vector,
                                                  const CorpusIndex& index) const {
    // TODO: return the ranked search results for the requested query.
    if(k < 0){ //When k is negative, throw bad argument
        throw std::invalid_argument("k is negative");
    }
    if(k == 0){ 
        return {}; //Returning empty vector
    }

    std::vector<std::string> query_terms_vector = unique_terms(query);
    if(query_terms_vector.empty()){ //If query is empty
        return {}; //return empty vector
    }
    std::vector<CandidateScore> candidates_vector = collect_candidates(query_terms_vector, index, chunk_vector.size());

    for(CandidateScore& single_candid : candidates_vector){ //Apply coverage then round final score
        double coverage = 1.0 + 0.10 * static_cast<double>(single_candid.match_terms) / static_cast<double>(query_terms_vector.size());
        double score = single_candid.b_score * coverage;
        single_candid.score = canonical_score(score);
    }
    std::sort(candidates_vector.begin(), candidates_vector.end(), [&](const CandidateScore& a, const CandidateScore& b){
        if(a.score != b.score){ //Higher scores first
            return a.score > b.score;
        }
        const Chunk& chunk_a = chunk_vector[a.chunk_index];
        const Chunk& chunk_b = chunk_vector[b.chunk_index];
        if(chunk_a.document_order != chunk_b.document_order){ //When tie, earlier the document insert order is wins
            return chunk_a.document_order <  chunk_b.document_order;
        }
        return chunk_a.sequence < chunk_b.sequence; //if from the same document, earlier chunk will win
    });
    std::vector<SearchResult> result_vector;
    std::size_t result_count = std::min(candidates_vector.size(), static_cast<std::size_t>(k));
    for(std::size_t i = 0; i < result_count; ++i){
        const CandidateScore& candidate = candidates_vector[i];
        const Chunk& chunk =chunk_vector[candidate.chunk_index];
        SearchResult result;
        result.chunk_id = chunk.id;
        result.document_id = chunk.document_id;
        result.chunk_sequence = chunk.sequence;
        result.text =  chunk.text;
        result.score = candidate.score;
        result.matched_terms = candidate.match_terms;
        result_vector.push_back(result);
    }
    return result_vector;
}

}  // namespace aiws

