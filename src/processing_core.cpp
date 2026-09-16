#include "aiws/processing_core.hpp"

namespace aiws {

struct ProcessingCore::Impl {
    // TODO: define the internal state used by the processing core.
};

ProcessingCore::ProcessingCore() : impl_(std::make_unique<Impl>()) { }

ProcessingCore::~ProcessingCore() = default;

ProcessingCore::ProcessingCore(ProcessingCore&&) noexcept = default;

ProcessingCore& ProcessingCore::operator=(ProcessingCore&&) noexcept = default;

std::string ProcessingCore::normalize(const std::string& text) {
    // TODO: return the normalized form of the input text.
    std::string result; //String to store the normalized version of the input string
    bool separator = false; 
    //Bool variable that tracks one/more separation characters 
    //and allows separation characters to be replaced by a single space
    for(unsigned char ch : text){ 
    if(ch >= 'A' && ch <= 'Z'){ //Conversion from uppercase to lowercase letters
        if(separator && !result.empty()){ //If separation character reads true
            result += ' '; //Add space
        }
        result += static_cast<char>(ch - 'A' +'a'); //Convert from uppercase to lowercase
        //Not using std::tolower as specification defines ascii-only
        separator = false;

    }else if((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')){ //If character is lowercase or a number
        //leave alone
        if(separator && !result.empty()){
            result += ' ';
        }
        result += static_cast<char>(ch);
        separator = false;
    }   else { //Others are separators
        if(!result.empty()){
            separator = true;
        }
    }
    }
    return result;
}

void ProcessingCore::rebuild(const Workspace&) {
    // TODO: rebuild the processing state from the workspace.
}

const std::vector<Chunk>& ProcessingCore::chunks() const noexcept {
    static const std::vector<Chunk> empty;

    // TODO: return the chunks currently stored by the processing core.
    return empty;
}

std::size_t ProcessingCore::chunk_count() const noexcept {
    // TODO: return the number of stored chunks.
    return 0;
}

std::size_t ProcessingCore::document_frequency(const std::string&) const {
    // TODO: return the document frequency for the requested term.
    return 0;
}

std::size_t ProcessingCore::term_frequency(const std::string&,
                                           const std::string&) const {
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
