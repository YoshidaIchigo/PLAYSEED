#include "ReferenceTravelSubsystem.h"
bool UReferenceTravelSubsystem::BeginPainting()
{
    if(State!=EReferenceTravelState::AtPainting)return false;
    State=EReferenceTravelState::EnteringPainting;return true;
}
bool UReferenceTravelSubsystem::FinishPainting()
{
    if(State!=EReferenceTravelState::EnteringPainting)return false;
    State=EReferenceTravelState::SelectingStar;return true;
}
