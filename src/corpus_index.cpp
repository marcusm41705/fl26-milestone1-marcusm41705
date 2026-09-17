#include "aiws/corpus_index.hpp"
#include "aiws/text_processor.hpp"
#include <stdexcept>
namespace aiws {

CorpusIndex::CorpusIndex(const std::vector<Chunk>& chunks) {
    build(chunks);
}

void CorpusIndex::build(const std::vector<Chunk>& chunks) {
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
    auto it = postings_.find(normal_term);
    if(it ==  postings_.end()){
        return 0;
    }
    return it->second.size();
}

std::size_t CorpusIndex::term_frequency(
    const std::string& normal_term,
    const std::string& chunk_id) const noexcept {
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
    auto it = postings_.find(normal_term);
    if(it == postings_.end()){
        return nullptr;
    }
    return &(it->second);
}

const Chunk* CorpusIndex::find_chunk(
    const std::vector<Chunk>& chunk_vector,
    const std::string& chunk_id) const noexcept {
    auto it = chunk_by_id_.find(chunk_id);
    if(it == chunk_by_id_.end()){
        return nullptr;
    }
    return &chunk_vector[it->second];
}

std::size_t CorpusIndex::chunk_index(const std::string& chunk_id) const {
auto it = chunk_by_id_.find(chunk_id);
if(it == chunk_by_id_.end()){
    throw std::out_of_range("chunk ID not found");
}
    return it->second;
}

}  // namespace aiws
