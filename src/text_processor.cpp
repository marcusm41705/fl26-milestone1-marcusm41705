#include "aiws/text_processor.hpp"

namespace aiws {
//Helper functions for tokenize
bool TokenChar(unsigned char c){
    //Will return true if character is an ASCII number or letter
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}
char toLowerCase(unsigned char c){
    if(c >= 'A' && c <= 'Z'){
        return static_cast<char>(c - 'A' + 'a');
    }
    return static_cast<char>(c);
}
bool ParagraphBreak(const std::string& text, std::size_t beginning, std::size_t ending){
    bool read_new_line = false;
    std::size_t i = beginning;
    while (i < ending){
        if(text[i] == '\r' && i + 1 < ending && text[i + 1 ] == '\n'){
            if(read_new_line){
            return true;
            } 
            read_new_line = true;
            i += 2;
        } else if (text[i] == '\n'){
            if(read_new_line){
                return true;
            }
            read_new_line = true;
            ++i;
        } else if(text[i] == ' ' || text[i] == '\t'){
            ++i;
        } else {
            read_new_line = false;
            ++i;
        }
    }
    return false;
}

std::vector<TokenInfo> TextProcessor::tokenize(const std::string& text) {
    // Produces the normalized tokens with source and paragraph information.
std::vector<TokenInfo> token_vector;
std::size_t i = 0;
std::size_t paragraph = 0;
bool readToken = false;
while (i < text.size()){ 
    std::size_t separatorCharBegin = i; //Index where the separator character region will begin
    while (i < text.size() && !TokenChar(static_cast<unsigned char>(text[i]))){
        ++i; //Skips all separator characters
    }
    if(readToken && ParagraphBreak(text, separatorCharBegin, i)){
        paragraph++; //Begin a new paragraph where a blamk line occurs between tokens
    }
    if(i >= text.size()){
        break;
    }
    std::size_t tokenBegin = i; //Recording where the single token begins
    std::string singleToken;
    while(i < text.size() && TokenChar(static_cast<unsigned char>(text[i]))){
        singleToken += toLowerCase(static_cast<unsigned char> (text[i]));
        ++i;
    }
    TokenInfo token_info;
    token_info.token = singleToken;
    token_info.begin = tokenBegin;
    token_info.end = i;
    token_info.paragraph = paragraph;
    token_vector.push_back(token_info);
    readToken = true;
}
    return token_vector;
}


std::vector<std::string> TextProcessor::terms(const std::string& text) {
    // Returns the normalized terms represented by the input text.
    std::vector<std::string> result_vector;
    std::vector<TokenInfo> token_vector = tokenize(text);
    for(const TokenInfo& single : token_vector){
        result_vector.push_back(single.token);
    }
    return result_vector;
}

std::string TextProcessor::normalize(const std::string& text) {
    
    std::vector<TokenInfo> token_vector = tokenize(text);
    return join(token_vector, 0, token_vector.size());
}

std::string TextProcessor::join(const std::vector<TokenInfo>& token_vector,
                                std::size_t beginning,
                                std::size_t end) {
    
    std::string resultString;
    for(auto i =  beginning; i < end; ++i){
        if(!resultString.empty()){
           resultString += ' ';
            
        }
        resultString += token_vector[i].token;
    }

    return resultString;
}

std::string TextProcessor::join(const std::vector<std::string>& token_vector,
                                std::size_t begin,
                                std::size_t end) {
    
    std::string resultString;
    for(auto i = begin; i < end; ++i){
        if(!resultString.empty()){
            resultString += ' ';
        }
        resultString += token_vector[i];
    }

    return resultString;
}

}  // namespace aiws
