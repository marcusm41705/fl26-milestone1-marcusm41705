#include "aiws/processing_core.hpp"
#include "aiws/text_processor.hpp"
#include "aiws/chunker.hpp"
#include "aiws/corpus_index.hpp"
#include "aiws/retrieval_engine.hpp"
#include "aiws/context_builder.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
int failures = 0;
void check(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}
std::string numbered_words(int n) {
    std::string s;
    for (int i = 0; i < n; ++i) {
        if (!s.empty()) s += ' ';
        s += "w" + std::to_string(i);
    }
    return s;
}
std::string range(int b, int e){
    std::string s;
    for(auto i =  b; i < e; ++i){
        if (!s.empty()) s+= ' ';
        s += "w" + std::to_string(i);
    }
    return s;
}
}
int main(){ 
    using namespace aiws;
    //-
    //TextProcessor testing
    //-
    check(TextProcessor::normalize("Hello, WORLD!") == "hello world", 
    "normalize converts uppercase ASCII + removes punctuation");

    check(TextProcessor::normalize("ECE-3544") == "ece 3544",
     "normalize retains both letters and numbers and recognizes punctuation as separators");
    
    check(TextProcessor::normalize(":)?!?!") == "", 
    "normalize makes a string with only punctuation into an empty string");
   
    check(TextProcessor::normalize("Marcus---Mason____Code")== "marcus mason code", 
    "multiple separators in consecutive order result in only one space");
    //Paragraph handling
    auto lf_tokens = TextProcessor::tokenize("marcus mason\n   \t\nmarcus mason");
    check(lf_tokens.size() == 4, "tokenize will find four tokens across two paragraphs");

    check(lf_tokens.size() == 4 && lf_tokens[0].paragraph == 0 && lf_tokens[1].paragraph == 0 && lf_tokens[2].paragraph == 1 
    && lf_tokens[3].paragraph == 1, "LF blank line creates a new paragraph");
    //CRLF handling
    auto crlf_token = TextProcessor::tokenize("marcus\r\n\r\nmason");
    check(crlf_token.size() == 2 && crlf_token[0].paragraph == 0 && crlf_token[1].paragraph == 1, 
    "A CRLF blank line will create a new paragraph");
    auto source_token =  TextProcessor::tokenize("Hello, WORLD!");
    check( source_token.size() == 2 && source_token[0].begin == 0 && source_token[0].end == 5 && 
    source_token[1].begin == 7 && source_token[1].end == 12,
    "the tokens' source position will refer to the original input. ");
    //-
    //Chunker testing
    //-
    Chunker chunker;
    Document d0{"test_chunk", "Marcus", numbered_words(120)};
    auto ex_chunks = chunker.chunk(d0, 0);
    check(ex_chunks.size() == 1 && ex_chunks[0].token_count == 120,
     "exactly 120 tokens will create one chunk");
     //121 tokens
     Document d1{"overlap", "Overlapping", numbered_words(121)};
     auto overlap_ch = chunker.chunk(d1, 0);
     check(overlap_ch.size() == 2, "121 tokens will result in two chunks");

     check (overlap_ch.size() == 2 && overlap_ch[0].token_count == 120 && overlap_ch[1].token_count == 21,
      "A 121 token document will use the 20-token overlap");
      if(overlap_ch.size() == 2){
        auto first_t = TextProcessor::terms(overlap_ch[0].text);
        auto second_t = TextProcessor::terms(overlap_ch[1].text);
        check(first_t.size() > 100 && !second_t.empty() && first_t[100] == second_t[0], 
        "second chunk will begin at 20 tokens before the first chunk will end");
      }
      //Paragraph preference
      std::string pg_text = range(0,110) + "\n\n" + range(110,150);
      Document d2{"paragraph", "Paragraph", pg_text};
      auto pg_chunks = chunker.chunk(d2, 0);
      check(!pg_chunks.empty() && pg_chunks[0].token_count == 110, 
      "chunk will prefer pg boundary inside the positions 100-120");
      //Empty document
      Document d3{"empty", "Empty", "?!?! --- !?!?"};
      auto d3_ch = chunker.chunk(d3, 0);
      check(d3_ch.empty(), 
      "empty document will provide no chunks");
      //-
      //Corpus Index Testing
      //-
      Chunk c1;
      c1.id = "a#0";
      c1.document_id = "a";
      c1.text = "ford ford dodge";
      Chunk c2;
      c2.id = "b#0";
      c2.document_id = "b";
      c2.text = "ford tesla";
      std::vector<Chunk> index_chunk_vector{c1, c2};
      CorpusIndex index(index_chunk_vector);

      check(index.document_frequency("ford") == 2,
    "the document frequency will count chunks that contain the term");

      check(index.document_frequency("dodge") == 1, 
      "document frequency will count the chunk that contains the term");

      check(index.document_frequency("test") == 0, 
    "unknown index term will return document frequency 0");

    check(index.term_frequency("ford", "a#0") == 2, 
    "term frequency will count the repeated instances of a term inside a single chunk");

    check(index.term_frequency("marcus", "a#0") == 0,
    "term frequency should be zero when the term is not inside the chunk");

    check(index.term_frequency("ford", "test#0") == 0, 
    "incorrect chunk ID will have a term frequency of 0");
    //-
    //RetrievalEngine
    //-
    c1.document_order = 0;
    c1.sequence = 0;
    c2.document_order = 1;
    c2.sequence = 0;
    index_chunk_vector = {c1, c2};
    CorpusIndex retrieval_index(index_chunk_vector);
    RetrievalEngine re;
    auto once = re.search("ford", 10, index_chunk_vector, retrieval_index);
    auto repeated = re.search("ford ford ford", 10, index_chunk_vector , retrieval_index);
    check(once.size() == repeated.size(), 
    "repeated query terms will not change the result count");
    bool equal_results = once.size() == repeated.size();
    if(equal_results){
        for (std::size_t i = 0; i < once.size();++i){
            if(once[i].chunk_id != repeated[i].chunk_id || once[i].score != repeated[i].score){
                equal_results = false;
                break;
            }
        }
    }

    check(equal_results, 
        "repeated query terms will not alter the ranking nor the score");
    //Unknown terms
    auto unknown = re.search("testunknown", 10, index_chunk_vector, retrieval_index);
    check(unknown.empty(), "query that contains unknown terms will return no candidates");
    //-
    //ContextBuilder
    //
    SearchResult sr;
    sr.chunk_id = "context#0";
    sr.document_id = "context";
    sr.chunk_sequence = 0;
    sr.text = "marcus mason computer engineering vt";
    sr.score = 2.0;
    ContextBuilder builder;
    //No/Zero budget
    auto no_ct = builder.build({sr}, 0);
    check(no_ct.empty(), "no/zero context budget will return no items");
   //Exact fit
   auto exact_ct = builder.build({sr}, 5);
   check(exact_ct.size() == 1 && exact_ct[0].token_count == 5 && !exact_ct[0].truncated,
    "An item with exact fit context item will not be truncated");
    //Truncation
    auto truncated_ct = builder.build({sr}, 3);
    check(truncated_ct.size() == 1 && truncated_ct[0].text == "marcus mason computer" && truncated_ct[0].token_count == 3
    && truncated_ct[0].truncated, "context will use the largest token prefix when the result doesn't fit");
    //Duplicate prevention testing
    std::vector<SearchResult> duplicate_vector{sr, sr};
    auto duplicate_ct = builder.build(duplicate_vector,10);
    check(duplicate_ct.size() == 1, "ContextBuilder will not include the duplicate chunk ID");
    //-
    //Processing Core
    //
    //Rebuild testing
    Workspace ws;
    ws.add_document(Document{"correct", "Correct", "washington commanders"});
    ProcessingCore c;
    c.rebuild(ws);
    check(c.chunk_count() == 1, "A successful and valid rebuild will create an expected corpus");
    Workspace ws_invalid;
    ws_invalid.add_document(Document{"duplicate", "Philadelphia", "philadelphia eagles"});
    ws_invalid.add_document(Document{"duplicate", "Dallas", "cowboys"});
    bool duplicate_throw = false;
    try{
        c.rebuild(ws_invalid);
    } catch(const std::invalid_argument&) {
        duplicate_throw = true;
    }
    check(duplicate_throw, "documents with the same IDs will throw invalid_argument");
    check(c.chunk_count() == 1 && c.document_frequency("washington") == 1 && c.document_frequency("philadelphia") == 0, 
    "valid corpus are preserved upon a failed rebuild");
    //repeated rebuild
    c.rebuild(ws);
    c.rebuild(ws);
    check(c.chunk_count() == 1, "duplicate state is not accumulated upon a repeated rebuild");

    if (failures == 0) {
        std::cout << "All student tests passed.\n";
        return 0;
    }
    std::cerr << failures << " student test(s) failed.\n";
    
    return 1;
}