#pragma once
#include "../../libraries/ExpressionEvaluator/ExpressionEvaluator.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <array>
namespace Compyler{
    struct NFAMapping
    {
        ulong mInitialState;
        ulong mTargetState;
        char mMappingSymbol;
    };
    struct NFASpec
    {
        std::vector<ulong> sStates;
        ulong sStartState = -1;
        ulong sHultState = -1;
        std::array<char, 30> sAlphabet;
        std::vector<NFAMapping> sTransitionFunctions;
    };
    static ulong maxState = 0;
    class NFA
    {
        public:
            NFA() = default;
            std::vector<NFAMapping> findMapByInitialState(ulong state);
            NFA convertFromRegEx(std::string& regEx);
            void followEpsilon(char mappingSymb, int index, std::vector<ulong>& States);
            void pushNewHult(char mapping = '\0');
            void pushNewStart(char mapping = '\0');
            void addState(NFAMapping& map);
            friend std::ostream& operator<<(std::ostream& out, NFA& nfa);
        private:
            std::vector<ulong> mStates;
            ulong mStartState;
            ulong mHultState;
            std::array<char, 30> mAlphabet;
            std::vector<NFAMapping> mTransitionFunctions;
        private:
            ExpressionSpecs<NFA, char> expSpecs;
            int findByInitialState(ulong state, bool first = true);

    };
}
