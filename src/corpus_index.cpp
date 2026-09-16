#include "aiws/corpus_index.hpp"
#include "aiws/text_processor.hpp"
namespace aiws {

CorpusIndex::CorpusIndex(const std::vector<Chunk>& chunks) {
    build(chunks);
}

void CorpusIndex::build(const std::vector<Chunk>& chunks) {
    // TODO: build the searchable index from the supplied chunks.
postings_.clear(); //Remove info from previous corpus
chunk_by_id_.clear();
for(std::size_t i = 0; i < chunks.size(); ++i)
{
    const Chunk& chunk = chunks[i];
    chunk_by_id_[chunk.id] =i; //This stores location of chunk inside the vector
    std::unordered_map<std::string,std::size_t> frequencies;
    std::vector<std::string> term_vector = TextProcessor::terms(chunk.text);
    for(const std::string& t:term_vector){
        ++frequencies[t];
    }
    for(const auto& e : frequencies){ //Add single posting for each term in chunk
        Posting p;
        p.chunk_index = i;
        p.frequency = e.second;
        postings_[e.first].push_back(p);
    }
}
}

std::size_t CorpusIndex::document_frequency(
    const std::string& normal_term) const noexcept {
    // TODO: return how many chunks contain the requested term.
    auto it = postings_.find(normal_term);
    if(it ==  postings_.end()){
        return 0;
    }
    return it->second.size();
}

std::size_t CorpusIndex::term_frequency(
    const std::string& normal_term,
    const std::string& chunk_id) const noexcept {
    // TODO: return the requested term's frequency in the specified chunk.
    auto c_it = chunk_by_id_.find(chunk_id);
    if(c_it == chunk_by_id_.end()){
        return 0;
    }
    auto p_it = postings_.find(normal_term);
    if(p_it == postings_.end()){
        return 0;
    }
    std::size_t intent_index =c_it->second;
    for(const Posting& posting : p_it->second){
        if(posting.chunk_index == intent_index){
            return posting.frequency;
        }

    }
    
    return 0;
}

const std::vector<CorpusIndex::Posting>* CorpusIndex::postings(
    const std::string& normal_term) const noexcept {
    // TODO: return the postings associated with the requested term.
    auto it = postings_.find(normal_term);
    if(it == postings_.end()){
        return nullptr;
    }
    return &(it->second);
}

const Chunk* CorpusIndex::find_chunk(
    const std::vector<Chunk>&,
    const std::string&) const noexcept {
    // TODO: find the chunk identified by the requested chunk ID.
    return nullptr;
}

std::size_t CorpusIndex::chunk_index(const std::string&) const {
    // TODO: return the stored index of the requested chunk ID.
    return 0;
}

}  // namespace aiws
