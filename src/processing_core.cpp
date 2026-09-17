#include "aiws/processing_core.hpp"
#include "aiws/text_processor.hpp"
#include "aiws/corpus_index.hpp"
#include "aiws/chunker.hpp"
#include "aiws/retrieval_engine.hpp"
#include "aiws/context_builder.hpp"
#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include <cmath>

namespace aiws {


struct ProcessingCore::Impl {
    // Defines the internal state used by the processing core.
    std::vector<Chunk> chunks_;
    CorpusIndex corpus_index_;
};

ProcessingCore::ProcessingCore() : impl_(std::make_unique<Impl>()) { }

ProcessingCore::~ProcessingCore() = default;

ProcessingCore::ProcessingCore(ProcessingCore&&) noexcept = default;

ProcessingCore& ProcessingCore::operator=(ProcessingCore&&) noexcept = default;

std::string ProcessingCore::normalize(const std::string& text) {
    //Returns the normalized form of the input text.
    return TextProcessor::normalize(text);
}

void ProcessingCore::rebuild(const Workspace& workspace) {
    // Rebuilds the processing state from the workspace.
 std::vector<Chunk> new_chunk_vector;
std::unordered_set<std::string> doc_ids;
Chunker chunker;
const auto& documents = workspace.documents();
for (std::size_t i = 0; i < documents.size();++i){
    const Document& document = documents[i];
    if(!doc_ids.insert(document.id()).second){//Validate doc IDs so they are unique and in order
        throw std::invalid_argument("Uses a duplicate Document ID");
    }
    std::vector<Chunk> document_chunk_vec = chunker.chunk(document, i); //Chunk all docs in order, add to new_chunk_vector
    new_chunk_vector.insert(new_chunk_vector.end(), document_chunk_vec.begin(), document_chunk_vec.end());
    //Appending chunks to new_chunk_vector
}
 //Build new corpus index from new_chunk_vector
 CorpusIndex new_corpus_index(new_chunk_vector);
 //Replace old chunk vector and corpus index with new ones
 impl_->chunks_ = std::move(new_chunk_vector);
 impl_->corpus_index_ = std::move(new_corpus_index);

}

const std::vector<Chunk>& ProcessingCore::chunks() const noexcept {
    //static const std::vector<Chunk> empty;

    //Returns the chunks currently stored by the processing core.
    return impl_->chunks_;
    
}

std::size_t ProcessingCore::chunk_count() const noexcept {
    //Returns the number of stored chunks.
      return impl_->chunks_.size();
}

std::size_t ProcessingCore::document_frequency(const std::string& term) const {
    //Returns the document frequency for the requested term.
    std::vector<std::string> normal_vector = TextProcessor::terms(term);
    if(normal_vector.empty()){
        return 0;
    }
    if(normal_vector.size()> 1){
        throw std::invalid_argument("Term must be a normalized to a single token");
    }
    return impl_->corpus_index_.document_frequency(normal_vector[0]);
}

std::size_t ProcessingCore::term_frequency(const std::string& term,
                                           const std::string& chunk_id) const {
    //Returns the term frequency for the requested chunk.
    std::vector<std::string> normal_vector = TextProcessor::terms(term);
    if(normal_vector.empty()){
        return 0;
    }
    if(normal_vector.size()  > 1){
        throw std::invalid_argument("Term must be normalized to a single token");

    }
    return impl_->corpus_index_.term_frequency(normal_vector[0],chunk_id);
}

std::vector<SearchResult> ProcessingCore::search(const std::string& query, int k) const {
    // Returns the ranked results for the requested query.
    RetrievalEngine re;
    return re.search(query, k, impl_->chunks_, impl_->corpus_index_);
}


std::vector<ContextItem> ProcessingCore::build_context(const std::string& query, 
                                                       int k,
                                                       std::size_t token_budget) const {
    // Builds bounded context for the requested query.
    std::vector<SearchResult> ranked_vector = search(query, k); //Retrieve the ranked search results and store inside vector
    ContextBuilder builder; //Build the context from ranked results within token budget
    return builder.build(ranked_vector, token_budget);
}

}  // namespace aiws
