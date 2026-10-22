#pragma once

#include <SpellChess/Board.h>
#include <vector>
#include <string>
#include <optional>

namespace MoveGeneratorTester {

    namespace IceCream {
        enum SpellPolicy {
            Addable,
            Fixed
        };

        struct CandidateMove {
            SpellChessLib::SpellChessMove base;
            SpellPolicy policy;
        };
    };


    enum class AcceptanceCriteria {
        SetEquality,
        SetSubset,
        DisjoinedSets
    };

    using MoveContainer = typename std::vector<IceCream::CandidateMove>;

    inline std::string to_string(const IceCream::CandidateMove & move){
        std::string result = SpellChessLib::spellChessMoveToString(move.base);

        if (move.policy == IceCream::Addable)
            result += "-A";
        
        if (move.policy == IceCream::Fixed)
            result += "-F";

        return result;
    };

    template <class ForwardIterator>
    std::string moveSetToString(ForwardIterator begin, ForwardIterator end){
        std::string result = "{";

        while (begin != end) {
            result += to_string(*begin);
            ++begin;
            if (begin != end){
                result.push_back(',');
                result.push_back(' ');
            }
        };

        result += "}";
        return result;
    };

    inline std::string moveSetToString(const MoveContainer & moveSet) {
        return moveSetToString(moveSet.begin(), moveSet.end());
    };

    struct TestCase {
        AcceptanceCriteria criteria;
        MoveContainer moves;
        std::string positionFen;
    };

    extern std::vector<TestCase> loadTests();

    extern std::optional<std::string> equalityTest(const MoveContainer & actual, const MoveContainer & expected);
    extern std::optional<std::string> subsetTest(const MoveContainer & actual, const MoveContainer & expected);  
    extern std::optional<std::string> disjoinedTest(const MoveContainer & actual, const MoveContainer & expected);  

    extern std::optional<std::string> checkTestCase(MoveContainer actual, TestCase test); 
};