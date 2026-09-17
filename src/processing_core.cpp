#include "aiws/processing_core.hpp"
#include "aiws/text_processor.hpp"
#include "aiws/corpus_index.hpp"
#include "aiws/chunker.hpp"
#include "aiws/retrieval_engine.hpp"
#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include <cmath>

namespace aiws {


struct ProcessingCore::Impl {
    // TODO: define the internal state used by the processing core.
    std::vector<Chunk> chunks_;
    CorpusIndex corpus_index_;
};

ProcessingCore::ProcessingCore() : impl_(std::make_unique<Impl>()) { }

ProcessingCore::~ProcessingCore() = default;

ProcessingCore::ProcessingCore(ProcessingCore&&) noexcept = default;

ProcessingCore& ProcessingCore::operator=(ProcessingCore&&) noexcept = default;

std::string ProcessingCore::normalize(const std::string& text) {
    // TODO: return the normalized form of the input text.
    return TextProcessor::normalize(text);
}

void ProcessingCore::rebuild(const Workspace& workspace) {
    // TODO: rebuild the processing state from the workspace.
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

    // TODO: return the chunks currently stored by the processing core.
    return impl_->chunks_;
    
}

std::size_t ProcessingCore::chunk_count() const noexcept {
    // TODO: return the number of stored chunks.
      return impl_->chunks_.size();
}

std::size_t ProcessingCore::document_frequency(const std::string& term) const {
    // TODO: return the document frequency for the requested term.
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
    // TODO: return the term frequency for the requested chunk.
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
    // TODO: return the ranked results for the requested query.
    RetrievalEngine re;
    return re.search(query, k, impl_->chunks_, impl_->corpus_index_);
}


std::vector<ContextItem> ProcessingCore::build_context(const std::string& query, 
                                                       int k,
                                                       std::size_t token_budget) const {
    // TODO: build bounded context for the requested query.
    if(k < 0){
        throw std::invalid_argument("k cannot be negative");
    }
    if(token_budget == 0){
        return {};
    }
    std::vector<SearchResult> search_result_vector = search(query, k);
    std::vector<ContextItem> context_item_vector;
    std::size_t used_token = 0;
    for(const SearchResult& result : search_result_vector){
        //Locate OG chunk, then use stored token count and source information
        const Chunk* chunk = impl_->corpus_index_.find_chunk(impl_->chunks_, result.chunk_id);
        if(chunk == nullptr){
            continue;
        }
        std::size_t remain_bud = token_budget - used_token;
        if(chunk->token_count <= remain_bud){ //Entire chunk will fit in remaining budget
            ContextItem item;
            item.chunk_id = chunk->id;
            item.document_id = chunk->document_id;
            item.chunk_sequence = chunk->sequence;
            item.text = chunk->text;
            item.score = result.score;
            item.truncated = false;
            item.token_count = chunk->token_count;
            context_item_vector.push_back(item);
            used_token += chunk->token_count;
            
        } else { //Complete chunk does not fit in remaining budget
            if(remain_bud > 0){
                std::vector<std::string> term_vector = TextProcessor::terms(chunk->text);
                ContextItem item;
                item.chunk_id = chunk->id;
                item.document_id = chunk->document_id;
                item.chunk_sequence = chunk->sequence;
                item.text = TextProcessor::join(term_vector, 0, remain_bud);
                item.token_count = remain_bud;
                item.score = result.score;
                item.truncated = true;
                context_item_vector.push_back(item);
            }
            break; //Once partial chunk is included, then will stop the context construction
        }
        if(used_token == token_budget){
            break;
        }
    }
    return context_item_vector;
}

}  // namespace aiws
