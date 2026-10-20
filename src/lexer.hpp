#pragma once
#include <cctype>
#include <climits>
#include <string>
#include <unordered_map>
#include <vector>
#include "common.hpp"

enum class T {
    Num, Ident, Let, Print, If, Else, While,
    LParen, RParen, LBrace, RBrace, Semi, Assign,
    Plus, Minus, Star, Slash, Percent, Not,
    Eq, Ne, Lt, Le, Gt, Ge, And, Or, End
};

struct Token {
    T kind;
    std::string text;
    long long value = 0;
    int line = 1;
};

inline std::vector<Token> tokenize(const std::string& s) {
    static const std::unordered_map<std::string, T> keywords = {
        {"let", T::Let}, {"print", T::Print}, {"if", T::If},
        {"else", T::Else}, {"while", T::While}};
    std::vector<Token> out;
    size_t i = 0;
    int line = 1;
    auto push = [&](T k, std::string text, long long v = 0) {
        out.push_back({k, std::move(text), v, line});
    };
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '\n') { ++line; ++i; continue; }
        if (std::isspace(c)) { ++i; continue; }
        if (c == '#') {
            while (i < s.size() && s[i] != '\n') ++i;
            continue;
        }
        if (std::isdigit(c)) {
            size_t j = i;
            long long v = 0;
            while (j < s.size() && std::isdigit(static_cast<unsigned char>(s[j]))) {
                int d = s[j] - '0';
                if (v > (LLONG_MAX - d) / 10) throw CompileError("integer literal too large", line);
                v = v * 10 + d;
                ++j;
            }
            push(T::Num, s.substr(i, j - i), v);
            i = j;
            continue;
        }
        if (std::isalpha(c) || c == '_') {
            size_t j = i;
            while (j < s.size() && (std::isalnum(static_cast<unsigned char>(s[j])) || s[j] == '_')) ++j;
            std::string word = s.substr(i, j - i);
            auto it = keywords.find(word);
            push(it == keywords.end() ? T::Ident : it->second, word);
            i = j;
            continue;
        }
        char n = i + 1 < s.size() ? s[i + 1] : '\0';
        if (c == '=' && n == '=') { push(T::Eq, "=="); i += 2; continue; }
        if (c == '!' && n == '=') { push(T::Ne, "!="); i += 2; continue; }
        if (c == '<' && n == '=') { push(T::Le, "<="); i += 2; continue; }
        if (c == '>' && n == '=') { push(T::Ge, ">="); i += 2; continue; }
        if (c == '&' && n == '&') { push(T::And, "&&"); i += 2; continue; }
        if (c == '|' && n == '|') { push(T::Or, "||"); i += 2; continue; }
        T k;
        switch (c) {
            case '(': k = T::LParen; break;
            case ')': k = T::RParen; break;
            case '{': k = T::LBrace; break;
            case '}': k = T::RBrace; break;
            case ';': k = T::Semi; break;
            case '=': k = T::Assign; break;
            case '+': k = T::Plus; break;
            case '-': k = T::Minus; break;
            case '*': k = T::Star; break;
            case '/': k = T::Slash; break;
            case '%': k = T::Percent; break;
            case '!': k = T::Not; break;
            case '<': k = T::Lt; break;
            case '>': k = T::Gt; break;
            default:
                throw CompileError(std::string("unexpected character '") + static_cast<char>(c) + "'", line);
        }
        push(k, std::string(1, static_cast<char>(c)));
        ++i;
    }
    push(T::End, "");
    return out;
}
