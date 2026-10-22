#include "TestCase.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace {

using Json = nlohmann::json;

std::string jsonTypeError(const std::string & field, const Json & value) {
    return "Field '" + field + "' must be a string, but was " + value.type_name();
}

}

using namespace MoveGeneratorTester;

std::vector<TestCase> MoveGeneratorTester::loadTests() {
    const std::filesystem::path testFile =
        std::filesystem::path(__FILE__).parent_path() / "tests.json";
    std::ifstream input(testFile);

    if (!input) {
        throw std::runtime_error("Could not open Icecream test file: " + testFile.string());
    }

    Json records;
    try {
        input >> records;
    } catch (const Json::parse_error & error) {
        throw std::runtime_error(
            "Could not parse Icecream test file '" + testFile.string() + "': " + error.what());
    }

    if (!records.is_array()) {
        throw std::runtime_error("Icecream test file must contain a top-level JSON array");
    }

    std::vector<TestCase> result;
    result.reserve(records.size());

    for (std::size_t testIndex = 0; testIndex < records.size(); ++testIndex) {
        const Json & record = records.at(testIndex);
        const std::string context = "Icecream test " + std::to_string(testIndex);

        if (!record.is_object()) {
            throw std::runtime_error(context + " must be a JSON object");
        }

        for (const char * field : {"name", "criteria", "positionFen", "moves"}) {
            if (!record.contains(field)) {
                throw std::runtime_error(context + " is missing required field '" + field + "'");
            }
        }

        if (!record.at("criteria").is_string()) {
            throw std::runtime_error(context + " " + jsonTypeError("criteria", record.at("criteria")));
        }
        if (!record.at("name").is_string()) {
            throw std::runtime_error(context + " " + jsonTypeError("name", record.at("name")));
        }
        if (!record.at("positionFen").is_string()) {
            throw std::runtime_error(context + " " + jsonTypeError("positionFen", record.at("positionFen")));
        }
        if (!record.at("moves").is_array()) {
            throw std::runtime_error(context + " field 'moves' must be an array, but was " +
                                     record.at("moves").type_name());
        }

        AcceptanceCriteria criteria;
        const std::string criteriaValue = record.at("criteria").get<std::string>();
        if (criteriaValue == "exact match") {
            criteria = AcceptanceCriteria::SetEquality;
        } else if (criteriaValue == "legal") {
            criteria = AcceptanceCriteria::SetSubset;
        } else if (criteriaValue == "illegal" || criteriaValue == "forbidden move") {
            criteria = AcceptanceCriteria::DisjoinedSets;
        } else {
            throw std::runtime_error(context + " has unknown criteria '" + criteriaValue +
                                     "' (expected 'exact match', 'legal', 'illegal', or 'forbidden move')");
        }

        TestCase test{record.at("name").get<std::string>(), criteria, {}, record.at("positionFen").get<std::string>()};
        test.moves.reserve(record.at("moves").size());

        for (std::size_t moveIndex = 0; moveIndex < record.at("moves").size(); ++moveIndex) {
            const Json & moveRecord = record.at("moves").at(moveIndex);
            const std::string moveContext = context + ", move " + std::to_string(moveIndex);

            if (!moveRecord.is_object()) {
                throw std::runtime_error(moveContext + " must be a JSON object");
            }
            for (const char * field : {"move", "policy"}) {
                if (!moveRecord.contains(field)) {
                    throw std::runtime_error(moveContext + " is missing required field '" + field + "'");
                }
                if (!moveRecord.at(field).is_string()) {
                    throw std::runtime_error(moveContext + " " +
                                             jsonTypeError(field, moveRecord.at(field)));
                }
            }

            const std::string moveValue = moveRecord.at("move").get<std::string>();
            SpellChessLib::SpellChessMove move;
            try {
                move = SpellChessLib::parseMove(moveValue);
            } catch (const std::exception & error) {
                throw std::runtime_error(moveContext + " has invalid move '" + moveValue + "': " +
                                         error.what());
            }

            const std::string policyValue = moveRecord.at("policy").get<std::string>();
            IceCream::SpellPolicy policy;
            if (policyValue == "addable") {
                policy = IceCream::Addable;
            } else if (policyValue == "fixed") {
                policy = IceCream::Fixed;
            } else {
                throw std::runtime_error(moveContext + " has unknown policy '" + policyValue + "'");
            }

            test.moves.push_back({move, policy});
        }

        result.push_back(std::move(test));
    }

    return result;
}

class CandidateComparator {
    public:
    CandidateComparator() {};
    bool operator()(const IceCream::CandidateMove & m1, const IceCream::CandidateMove & m2) {
        if (m1.policy != m2.policy)
            return m1.policy < m2.policy;
        return m1.base < m2.base;
    }
};

std::optional<std::string> MoveGeneratorTester::equalityTest(const MoveContainer & actual, const MoveContainer & expected) {
    CandidateComparator comparator;
    MoveContainer diff;
    auto diff_inserter = std::inserter(diff, diff.begin());
    auto end = std::set_symmetric_difference(actual.begin(), actual.end(),
                                             expected.begin(), expected.end(),
                                             diff_inserter, comparator);

    if (!diff.empty()) {
        std::string result = "Expected equality of these move sets: ";
        result += moveSetToString(actual);
        result += " and ";
        result += moveSetToString(expected);
        return result;
    };

    return std::nullopt;
}

std::optional<std::string> MoveGeneratorTester::subsetTest(const MoveContainer & actual, const MoveContainer & expected) {
    CandidateComparator comparator;
    MoveContainer intersection;
    auto intersection_inserter = std::inserter(intersection, intersection.begin());
    auto end = std::set_intersection(expected.begin(), expected.end(),
                                     actual.begin(), actual.end(),
                                     intersection_inserter, comparator);

    if (intersection.size() != expected.size()) {
        std::string result = "Expected the following set ";
        result += moveSetToString(actual);
        result += " to be subset of ";
        result += moveSetToString(expected);
        return result;
    };

    return std::nullopt;
}

std::optional<std::string> MoveGeneratorTester::disjoinedTest(const MoveContainer & actual, const MoveContainer & expected) {
    CandidateComparator comparator;
    MoveContainer intersection;
    auto intersection_inserter = std::inserter(intersection, intersection.begin());
    auto end = std::set_intersection(actual.begin(), actual.end(),
                                     expected.begin(), expected.end(),
                                     intersection_inserter, comparator);

    if (!intersection.empty()) {
        std::string result = "Expected the following set ";
        result += moveSetToString(actual);
        result += " to be disjoint to ";
        result += moveSetToString(expected);
        return result;
    };

    return std::nullopt;
}

std::optional<std::string> MoveGeneratorTester::checkTestCase(MoveContainer actual, TestCase test) {
    CandidateComparator comparator;
    std::sort(actual.begin(), actual.end(), comparator);
    std::sort(test.moves.begin(), test.moves.end(), comparator);

    switch (test.criteria)
    {
    case AcceptanceCriteria::SetEquality:
        return equalityTest(actual, test.moves);
    
    case AcceptanceCriteria::SetSubset:
        return subsetTest(actual, test.moves);

    case AcceptanceCriteria::DisjoinedSets:
        return disjoinedTest(actual, test.moves);

    default:
        throw std::invalid_argument("Unrecognised test passing criteria");
    }
}