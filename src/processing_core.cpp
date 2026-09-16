#include "aiws/processing_core.hpp"
#include "aiws/text_processor.hpp"
#include "aiws/corpus_index.hpp"
#include <stdexcept>
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

void ProcessingCore::rebuild(const Workspace&) {
    // TODO: rebuild the processing state from the workspace.

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
    
    return 0;
}

std::vector<SearchResult> ProcessingCore::search(const std::string&, int) const {
    // TODO: return the ranked results for the requested query.
    return {};
}

std::vector<ContextItem> ProcessingCore::build_context(const std::string&,
                                                       int,
                                                       std::size_t) const {
    // TODO: build bounded context for the requested query.
    return {};
}

}  // namespace aiws
