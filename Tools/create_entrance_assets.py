"""Create only the small entrance map, preserving existing course assets."""
import unreal
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
path='/Game/Battlefield/Entrance'
if unreal.EditorAssetLibrary.does_asset_exist(path):
    if not levels.load_level(path):raise RuntimeError('Could not load entrance')
elif not levels.new_level(path):raise RuntimeError('Could not create entrance')
mode=unreal.load_class(None,'/Script/PLAYSEED.ReferenceEntranceGameMode')
if not mode:raise RuntimeError('Missing entrance class')
editor.get_editor_world().get_world_settings().set_editor_property('default_game_mode',mode)
if not levels.save_current_level():raise RuntimeError('Could not save entrance')
unreal.log('PAINTING ENTRANCE ASSET PASS')
