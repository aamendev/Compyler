#include "../headers/NFA.h"

namespace Compyler{

    NFA NFA::convertFromRegEx(std::string& regEx)
    {
        Operator<NFA> conc{'\0', 2, [](NFA nfa1, NFA nfa2)
            {
                for (int i : nfa1.mStates)
                    nfa2.mStates.push_back(i);
                for (auto i : nfa1.mTransitionFunctions)
                    nfa2.mTransitionFunctions.emplace_back(i);
                nfa2.mTransitionFunctions.emplace_back(NFAMapping{nfa1.mHultState, nfa2.mStartState, '\0'});
                nfa2.mStartState = nfa1.mStartState;
                return nfa2;
            }
        };
        Operator<NFA> kleene{'*', 3, [](NFA nfa, NFA nfa2)
            {
                nfa2.mTransitionFunctions.emplace_back(NFAMapping{nfa2.mHultState, nfa2.mStartState, '\0'});
                nfa2.pushNewStart();
                nfa2.pushNewHult();
                nfa2.mTransitionFunctions.emplace_back(NFAMapping{nfa2.mStartState, nfa2.mHultState, '\0'});
                return nfa2;
            }
            ,true
        };
        Operator<NFA> alt{'|', 1, [](NFA nfa1, NFA nfa2)
            {
                nfa2.pushNewStart();
                nfa1.pushNewHult();
                nfa2.mTransitionFunctions.emplace_back(NFAMapping{nfa2.mStartState, nfa1.mStartState, '\0'});
                nfa1.mStartState = nfa2.mStartState;
                nfa2.mTransitionFunctions.emplace_back(NFAMapping{nfa2.mHultState, nfa1.mHultState, '\0'});
                nfa2.mHultState = nfa1.mHultState;
                nfa2.mTransitionFunctions.insert(nfa2.mTransitionFunctions.end(),
                        nfa1.mTransitionFunctions.begin(), nfa1.mTransitionFunctions.end());
                return nfa2;
            }
        };
        std::vector<Operator<NFA> > ops{conc, alt, kleene};
        expSpecs.sOperators = ops;
        expSpecs.sConvertionFunc = [](char c)
        {
            NFA nfa;
            char d = c;

            nfa.mStartState =  maxState++;
            nfa.mHultState =  maxState++;
            nfa.mStates.push_back(nfa.mStartState);
            nfa.mStates.push_back(nfa.mHultState);
            nfa.mTransitionFunctions = std::vector{
                NFAMapping{nfa.mStartState, nfa.mHultState, d},
            };
            return nfa;
        };
        ExpressionEvaluator mExpressionEvaluator(expSpecs);
        return mExpressionEvaluator.evaluate(regEx);
    }
    void NFA::pushNewHult(char mapping)
    {
        ulong state = maxState++;
        mStates.push_back(state);
        mTransitionFunctions.emplace_back(NFAMapping{mHultState, state, mapping});
        mHultState = state;
    }
    void NFA::pushNewStart(char mapping)
    {

        ulong state = maxState++;
        mStates.push_back(state);
        mTransitionFunctions.emplace_back(NFAMapping{state, mStartState, mapping});
        mStartState = state;
    }
    bool isInWorkingList(std::vector<ulong> states
                        , std::vector< std::vector<ulong> > WorkingList)
    {
        return std::find(WorkingList.begin(), WorkingList.end(), states) != std::end(WorkingList);
    }
    std::vector<DFAMapping> NFA::constructDFA(NFA& nfa)
    {
        std::vector<DFAMapping> dfa;
        std::vector<char> symbols = {'a', 'b', 'c'};
        std::vector<std::vector<ulong> > WorkingSet;
        std::vector<ulong> nfaStates;
        nfaStates.push_back(nfa.mStartState);
        WorkingSet.emplace_back(nfaStates);
        bool firstSet = true;
        while (!WorkingSet.empty())
        {
            std::vector<ulong> currentStateSet = std::move(WorkingSet[0]);
            DFANode dfaNode = DFANode{DFAState++, currentStateSet};
            std::vector<ulong> storageSet = currentStateSet;
            WorkingSet.pop_back();
        for (char c : symbols)
        {
            for (ulong s : currentStateSet)
            {
                std::vector<ulong> stateSet{s};
                nfa.followEpsilon(c,stateSet);
                if (!stateSet.empty())
                    stateSet.erase(stateSet.begin());
                for (auto i : stateSet)
                    std::cout<<i<<',';
                std::cout<<'\n';
                if (!stateSet.empty()
                    && !isInWorkingList(stateSet, WorkingSet))
                {
                    std::cout<<"Pushed";
                    WorkingSet.emplace_back(stateSet);
                }
            }
            currentStateSet = storageSet;
        }
        firstSet = false;
        }
    }
    void removeDuplicates(std::vector<ulong>& states)
    {
        std::set<ulong> set(states.begin(), states.end());
        states.clear();
        states.resize(set.size());
        std::copy(set.begin(), set.end(), states.begin());
    }
    int NFA::findByInitialState(ulong state, bool first)
    {
        for (int i = 0; i < mTransitionFunctions.size(); i++)
        {
            if (mTransitionFunctions[i].mInitialState == state)
                if (first)
                    return i;
                else
                    first = true;
        }
        return -1;
    }
    void NFA::followEpsilon(char mappingSymb, std::vector<ulong>& States, int index)
    {
        ulong currentState = States[index];
        int j = findByInitialState(currentState);
        char currentSymb = mTransitionFunctions[j].mMappingSymbol;
        bool cond = false;
        if (j != -1 && ((index != 0 && currentSymb == '\0')
                    || (index == 0 && currentSymb == mappingSymb)) )
        {
            cond = true;
            ulong k = mTransitionFunctions[j].mTargetState;
                States.push_back(k);
                followEpsilon(mappingSymb, States, ++index);
        }
        j = findByInitialState(currentState, false);
        currentSymb = mTransitionFunctions[j].mMappingSymbol;
        if (j != -1 && ((index != 0 && currentSymb == '\0')
                    || (index == 0 && currentSymb == mappingSymb)) )
           {
               cond = true;
               ulong k = mTransitionFunctions[j].mTargetState;
                   States.push_back(k);
                   followEpsilon(mappingSymb, States, ++index);
           }
        return;
    }
    bool NFA::reachedThroughEpsilon(ulong state, std::vector<ulong>& States)
    {
        for (ulong i : States)
            if (state == i)
                return true;
        return false;

    }
    std::ostream& operator<<(std::ostream& os, NFA& nfa)
    {
        os << nfa.mStartState << '\n';
        for (auto i : nfa.mTransitionFunctions)
        {
            os << i.mInitialState << "("<< i.mMappingSymbol
                << ") -> " << i.mTargetState;
            os << '\n';
        }
        os << nfa.mHultState << '\n';
        return os;
    }
}
