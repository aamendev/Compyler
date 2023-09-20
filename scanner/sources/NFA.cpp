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
    void NFA::followEpsilon(char mappingSymb, int index, std::vector<ulong>& States)
    {
        ulong currentState = States[index];
        int j = findByInitialState(currentState);
        char currentSymb = mTransitionFunctions[j].mMappingSymbol;
        bool cond = false;
        if (j != -1 && (currentSymb == '\0' || currentSymb == mappingSymb))
        {
            cond = true;
            ulong k = mTransitionFunctions[j].mTargetState;
            States.push_back(k);
            followEpsilon(mappingSymb, ++index, States);
        }
        j = findByInitialState(currentState, false);
        currentSymb = mTransitionFunctions[j].mMappingSymbol;
         if (j != -1 && (currentSymb == '\0' || currentSymb == mappingSymb))
           {
           cond = true;
           ulong k = mTransitionFunctions[j].mTargetState;
           States.push_back(k);
           followEpsilon(mappingSymb, ++index, States);
           }
        return;
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
