#include "aiws/context_builder.hpp"
#include "aiws/text_processor.hpp"
#include <unordered_set>
namespace aiws {

std::vector<ContextItem> ContextBuilder::build(const std::vector<SearchResult>& ranked_vector, std::size_t token_budget) const {
    // Assembles ranked results into context items within the supplied token budget.
    std::vector<ContextItem> context_item_vector;
   
    if(token_budget == 0){
        return context_item_vector;
    }
   std::size_t used_token = 0; 
   std::unordered_set<std::string> used_chunk_ids_set;
   for(const SearchResult& result : ranked_vector){ //Results pre-ranked, so preserve current order
    if(!used_chunk_ids_set.insert(result.chunk_id).second){ //Stop from adding the same chunk more than once
        continue;
    }
    std::vector<std::string> terms = TextProcessor::terms(result.text);
    std::size_t token_count = terms.size();
    std::size_t remain_bud = token_budget - used_token;
    
    if(token_count <= remain_bud){ //Complete result fits inside remaining budget
        ContextItem item;
        item.chunk_id = result.chunk_id;
        item.document_id = result.document_id;
        item.chunk_sequence = result.chunk_sequence;
        item.text = result.text;
        item.token_count = token_count;
        item.score = result.score;
        item.truncated = false;
        context_item_vector.push_back(item);
        used_token += token_count;
    } else {
        if(remain_bud > 0){ //Include largest token prefix that fits
            ContextItem item;
            item.chunk_id = result.chunk_id;
            item.document_id = result.document_id;
            item.chunk_sequence =  result.chunk_sequence;
            item.text = TextProcessor::join(terms, 0, remain_bud);
            item.token_count = remain_bud;
            item.score = result.score;
            item.truncated = true;
            context_item_vector.push_back(item);
        }
        break; //Stop after if first result isn't able to fit
        
    }
    if(used_token == token_budget){//Break if budget is entirely consumed
            break;
        }

   }
   return context_item_vector;
}

}  // namespace aiws
