#pragma once
#include <set>
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
    struct DFANode
    {
        ulong stateNumber;
        std::vector<ulong> Set;
    };
    struct DFAMapping
    {
        DFANode mInitialSet;
        DFANode mTargetSet;
        char mMappingSymbol;
    };
    static ulong DFAState = 0;
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
            void followEpsilon(char mappingSymb, std::vector<ulong>& States,int index = 0);
            friend std::ostream& operator<<(std::ostream& out, NFA& nfa);
            std::vector<DFAMapping> constructDFA(NFA& nfa);

        public:
            ulong startState() const { return mStartState; }
            ulong hultState() const { return mHultState; }
            std::vector<ulong> states() { return mStates; }
            std::array<char, 30> alphabet() { return mAlphabet;}
            std::vector<NFAMapping> mappings() { return mTransitionFunctions; }

        private:
            std::vector<ulong> mStates;
            ulong mStartState;
            ulong mHultState;
            std::array<char, 30> mAlphabet;
            std::vector<NFAMapping> mTransitionFunctions;
            ExpressionSpecs<NFA, char> expSpecs;
            friend void removeDuplicates(std::vector<ulong>& states);
            friend bool isInWorkList(std::vector<ulong>& states
                    , std::vector<std::vector<ulong> >& workingList);
        private:
            void pushNewHult(char mapping = '\0');
            void pushNewStart(char mapping = '\0');
            void addState(NFAMapping& map);
            int findByInitialState(ulong state, bool first = true);
            bool reachedThroughEpsilon(ulong state, std::vector<ulong>& States);
    };
}
