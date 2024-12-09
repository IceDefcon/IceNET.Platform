#include <stdio.h>
#include <stdbool.h>
 
// Define State IDs
typedef enum
{
    STATE_ID_0,
    STATE_ID_1,
    STATE_ID_2,
    STATE_ID_3,
} StateIdType;
 
// Function Pointer Types
typedef bool (*PermisionFunctionType)(void);
typedef bool (*ActionFunctionType)(StateIdType prevStateId, StateIdType newStateId);
 
// Transition Block Struct
typedef struct
{
    PermisionFunctionType permissionFunction;   // Pointer to a permission-checking function
    char* permissionFunctionName;               // Name of the permission function
    ActionFunctionType actionFunction;          // Pointer to an action function
    char* actionFunctionName;                   // Name of the action function
    StateIdType targetState;                    // Target state
} TransitionBlock;
 
// Permission Function
bool AllowTransition(void)
{
    // Always allow transitions
    return true;
}
 
// Action Function
bool TransitionAction(StateIdType prevState, StateIdType newState)
{
    // Simulate an action during state transition
    printf("Transitioning from %d to %d\n", prevState, newState);
    return true;
}
 
// Transition Table
TransitionBlock transitionTable[] = {
    { AllowTransition, "AllowTransition", TransitionAction, "TransitionAction", STATE_ID_1 },
    { AllowTransition, "AllowTransition", TransitionAction, "TransitionAction", STATE_ID_2 },
    { AllowTransition, "AllowTransition", TransitionAction, "TransitionAction", STATE_ID_3 },
    { AllowTransition, "AllowTransition", TransitionAction, "TransitionAction", STATE_ID_0 }
};
 
// Current State
StateIdType currentState = STATE_ID_0;
 
// State Machine Function
void RunStateMachine(void)
{
    for (int i = 0; i < sizeof(transitionTable) / sizeof(TransitionBlock); i++)
    {
        TransitionBlock transition = transitionTable[i];
        if (transition.targetState != currentState)
        {
            // Check permission
            if (transition.permissionFunction())
            {
                // Execute action
                if (transition.actionFunction(currentState, transition.targetState))
                {
                    // Update state
                    printf("State changed: %d -> %d\n", currentState, transition.targetState);
                    currentState = transition.targetState;
                }
                else
                {
                    printf("Action failed during transition.\n");
                }
            }
            else
            {
                printf("Transition not allowed.\n");
            }
        }
    }
}
 
// Main Function
int main()
{
    // Run the state machine
    for (int i = 0; i < 5; i++) // Run it 5 cycles for demonstration
    {
        RunStateMachine();
    }
    return 0;
}

