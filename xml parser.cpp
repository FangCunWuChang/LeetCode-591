#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <stack>
#include <map>
#include <cmath>
#include <queue>
#include <array>
#include <numeric>
using namespace std;

// TAG_NAME := (A-Z)(In 9 letters)
// StartTag := <TAG_NAME>
// EndTag := </TAG_NAME>
// CDATA_CONTENT := *(Not ']]>')
// CData := <![CDATA[CDATA_CONTENT]]>
// CONTENT := *(Not '<')
// Content := Content CData | Content Closure | CONTENT
// Content1 := CData Content | Closure Content | Content
// Closure := StartTag Content1 EndTag

class Solution
{
public:
    enum TokenType
    {
        StartTag,
        EndTag,
        Content,
        CData,
        Unknown
    };
    using Token = std::tuple<const char *, int, TokenType>;

    void skip(const char **p)
    {
        while ((**p) == ' ')
        {
            (*p)++;
        }
    }
    Token parseStartTag(const char **p)
    {
        (*p)++; // skip'<'

        const char *start = (*p);
        while ((**p) != '>' && (**p) != '\0')
        {
            (*p)++;
        }

        if ((**p) == '\0' || (*p) == start)
        {
            return {start, 0, Unknown};
        }

        (*p)++; // skip'>'
        return {start, (int)((*p) - start - 1), StartTag};
    }
    Token parseEndTag(const char **p)
    {
        (*p) += 2; // skip'</'

        const char *start = (*p);
        while ((**p) != '>' && (**p) != '\0')
        {
            (*p)++;
        }

        if ((**p) == '\0' || (*p) == start)
        {
            return {start, 0, Unknown};
        }

        (*p)++; // skip'>'
        return {start, (int)((*p) - start - 1), EndTag};
    }
    Token parseCData(const char **p)
    {
        (*p) += 9; // skip'<![CDATA['

        const char *start = (*p);
        while ((**p) != '\0')
        {
            if ((**p) == ']' && (*((*p) + 1)) == ']' && (*((*p) + 2)) == '>')
            {
                break;
            }
            (*p)++;
        }

        if ((**p) == '\0')
        {
            return {start, 0, Unknown};
        }

        (*p) += 3; // skip']]>'
        return {start, (int)((*p) - start - 3), CData};
    }
    Token parseContent(const char **p)
    {
        const char *start = (*p);
        while ((**p) != '<' && (**p) != '\0')
        {
            (*p)++;
        }

        if ((*p) == start)
        {
            return {start, 0, Unknown};
        }

        return {start, (int)((*p) - start), Content};
    }
    vector<Token> lexer(const string &code)
    {
        vector<Token> result;

        const char *ptr = code.data();

        while (*ptr != '\0')
        {
            skip(&ptr);

            if (*ptr == '<')
            {
                if (*(ptr + 1) == '/')
                {
                    result.push_back(parseEndTag(&ptr));
                    continue;
                }
                if (*(ptr + 1) == '!' && *(ptr + 2) == '[' &&
                    *(ptr + 3) == 'C' && *(ptr + 4) == 'D' && *(ptr + 5) == 'A' &&
                    *(ptr + 6) == 'T' && *(ptr + 7) == 'A' && *(ptr + 8) == '[')
                {
                    result.push_back(parseCData(&ptr));
                    continue;
                }
                result.push_back(parseStartTag(&ptr));
                continue;
            }

            result.push_back(parseContent(&ptr));
        }

        return result;
    }

    struct ASTNode
    {
        enum Type
        {
            Closure,
            Content,
            CData
        };
        Type type;
        const char *name;
        int nameLength;

        ASTNode *parent = nullptr;
        vector<ASTNode *> children;
    };

    bool checkTagName(const char *data, int length)
    {
        if (length > 9)
        {
            return false;
        }
        for (int i = 0; i < length; i++)
        {
            if (!(data[i] >= 'A' && data[i] <= 'Z'))
            {
                return false;
            }
        }
        return true;
    }
    bool compareTagName(const char *d1, int l1, const char *d2, int l2)
    {
        if (l1 != l2)
        {
            return false;
        }
        for (int i = 0; i < l1; i++)
        {
            if (d1[i] != d2[i])
            {
                return false;
            }
        }
        return true;
    }

    ASTNode *createAST(const vector<Token> &tokens)
    {
        ASTNode *root = nullptr, *current = nullptr;

        for (auto &[ptr, length, type] : tokens)
        {
            if (root == nullptr)
            {
                if (type != StartTag)
                {
                    return nullptr;
                }

                if (!checkTagName(ptr, length))
                {
                    return nullptr;
                }

                root = new ASTNode{ASTNode::Closure, ptr, length};
                current = root;
                continue;
            }

            if (!current)
            {
                return nullptr;
            }

            if (type == Unknown)
            {
                return nullptr;
            }
            if (type == Content)
            {
                current->children.push_back(new ASTNode{ASTNode::Content, ptr, length, current});
                continue;
            }
            if (type == CData)
            {
                current->children.push_back(new ASTNode{ASTNode::CData, ptr, length, current});
                continue;
            }
            if (type == StartTag)
            {
                if (!checkTagName(ptr, length))
                {
                    return nullptr;
                }
                auto node = new ASTNode{ASTNode::Closure, ptr, length, current};
                current->children.push_back(node);
                current = node;
                continue;
            }
            if (type == EndTag)
            {
                if (!compareTagName(current->name, current->nameLength, ptr, length))
                {
                    return nullptr;
                }
                current = current->parent;
            }
        }

        if (current)
        {
            return nullptr;
        }
        return root;
    }

    bool isValid(string code)
    {
        auto tokens = lexer(code);
        auto ast = createAST(tokens);
        return ast;
    }
};

int main()
{
    Solution s;
    string code = "<A><A>456</A>  <A> 123  !!  <![CDATA[]]>  123 </A>   <A>123</A></A>";
    s.isValid(code);
}