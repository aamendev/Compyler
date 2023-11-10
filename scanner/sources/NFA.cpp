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

    int isInDFA(std::vector<ulong> states, std::vector<DFAMapping> dfa)
    {
        for (int i = 0; i < dfa.size(); i++)
        {
            std::vector<ulong> initialStates = dfa[i].mInitialSet.Set;
            std::vector<ulong> finalStates = dfa[i].mTargetSet.Set;
            if (states == initialStates)
            {
                return dfa[i].mInitialSet.stateNumber;
            }
            else if (states == finalStates)
            {
                return dfa[i].mTargetSet.stateNumber;
            }
        }
        return -1;
    }

    std::vector<DFAMapping> NFA::constructDFA(NFA& nfa)
    {
        std::vector<DFAMapping> dfa;

        std::vector<char> symbols = {'a', 'b', 'c'};

        std::vector<std::vector<ulong> > WorkingSet;
        std::vector<std::vector<ulong> > visitedSets;

        std::vector<ulong> currentCycleNfaStates;

        currentCycleNfaStates.push_back(nfa.mStartState);
        WorkingSet.emplace_back(currentCycleNfaStates);
        visitedSets = WorkingSet;

        while (!WorkingSet.empty())
        {
            std::cout<<"DFA Size: "<<dfa.size()<<'\n';
            std::cout<<"Num Loop: "<< WorkingSet.size() << '\n';
            int temp = DFAState;


            std::vector<ulong> currentDfaStates = std::move(WorkingSet[WorkingSet.size() - 1]);

            DFANode dfaNode = DFANode{DFAState, currentDfaStates};

            std::vector<ulong> storageSet = currentDfaStates;

            WorkingSet.pop_back();
            for (char c : symbols)
            {
                std::vector<ulong> eSets = {};

                for (ulong s : currentDfaStates)
                {
                    eSets = {s};
                    nfa.followEpsilon(c,eSets);
                    if (!eSets.empty())
                        eSets.erase(eSets.begin());

                    /*Debug*/
                    for (auto i : eSets)
                        std::cout<<i<<',';
                    std::cout<<'\n';
                    /*End Debug */

                    if (!eSets.empty())
                    {
                        /* Log */
                        std::cout<<DFAState<<'\n';
                        /* End Log */
                        if (!isInWorkingList(eSets ,visitedSets))
                        {
                            std::cout<<"Pushed";
                            std::cout<<"\nIIIIIIIIIIIIIN\n";
                            WorkingSet.emplace_back(eSets);
                            visitedSets.emplace_back(eSets);
                        }
                        int temp2 = DFAState;
                        int cond = isInDFA(eSets, dfa);
                        DFAState = cond * (cond != -1) + (temp2 + 1) * (cond == -1);
                        DFANode mapNode{DFAState, eSets};
                        dfa.emplace_back(DFAMapping{dfaNode, mapNode, c});
                    }
                }
                currentDfaStates = storageSet;
            }
            DFAState = temp + 1;
        }
        return dfa;
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
