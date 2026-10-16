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
bool UReferenceTravelSubsystem::BeginCourse()
{
    if(!bEntranceFlow)return true;
    if(State!=EReferenceTravelState::SelectingStar)return false;
    State=EReferenceTravelState::Dropping;return true;
}
bool UReferenceTravelSubsystem::FinishDrop()
{
    if(State!=EReferenceTravelState::Dropping)return false;
    State=EReferenceTravelState::InCourse;return true;
}
bool UReferenceTravelSubsystem::BeginReturn()
{
    if(!bEntranceFlow||State==EReferenceTravelState::Returning)return false;
    State=EReferenceTravelState::Returning;return true;
}
bool UReferenceTravelSubsystem::IsTransitionLocked() const
{
    return State==EReferenceTravelState::EnteringPainting||State==EReferenceTravelState::Dropping||State==EReferenceTravelState::Celebrating||State==EReferenceTravelState::Returning;
}
