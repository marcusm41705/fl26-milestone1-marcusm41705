#include "aiws/chunker.hpp"
#include "aiws/text_processor.hpp"
#include <stdexcept>

namespace aiws {

Chunker::Chunker(ChunkingPolicy policy) : policy_(policy) {
    if (policy_.max_tokens == 0 || policy_.overlap >= policy_.max_tokens ||
        policy_.paragraph_window > policy_.max_tokens) {
        throw std::invalid_argument("invalid chunking policy");
    }
}

std::vector<Chunk> Chunker::chunk(const Document& document, std::size_t document_order) const {
  
    // TODO: produce deterministic, source-attributed chunks for the supplied document.
      std::vector<Chunk> chunk_vector;
      std::vector<TokenInfo> token_vector = TextProcessor::tokenize(document.text());
      if(token_vector.empty()){
        return chunk_vector;
      }
      std::size_t beginning = 0;
      std::size_t sequence = 0;
      while(beginning < token_vector.size()){
        std::size_t remaining = token_vector.size() - beginning;
        std::size_t end;
        if(remaining <= policy_.max_tokens){ //If remaining text will fit into a singular chunk, then keep all of it
            end =  token_vector.size();
            //This will force a default value to the maximum allowed chunk size
        } else {
            end = beginning + policy_.max_tokens;
            //Represents the earliest preferred paragraph boundary
            std::size_t preferred_begin = end - policy_.paragraph_window;
            for(std::size_t i = end; i >= preferred_begin; --i){ 
                //Start the search backwards for the most recent paragraph boundary
                //When the paragraph number changes, it means theres a paragraph boundary inside both tokens
                if(i < token_vector.size() && i > beginning && token_vector[i - 1].paragraph != token_vector[i].paragraph){
                    end = i;
                    break;
                }
                if(i == preferred_begin){
                    break; //Protection
                }
            }

        }
        Chunk c;
        c.document_id = document.id();
        c.document_order = document_order;
        c.sequence = sequence;
        c.id = document.id() + "#"  + std::to_string(sequence);
        c.text = TextProcessor::join(token_vector, beginning, end);
        c.token_count = end - beginning;
        c.source_begin = token_vector[beginning].begin; //Indexes that refer to OG text of the document
        c.source_end = token_vector[end - 1].end;
        chunk_vector.push_back(c);
        ++sequence;
        if(end == token_vector.size()){ //If final chunk, then finish 
            break;
        }
        beginning = end - policy_.overlap; //Set the beginning of the next chunk with overlap
      }
      return chunk_vector;
    
}

}  // namespace aiws
