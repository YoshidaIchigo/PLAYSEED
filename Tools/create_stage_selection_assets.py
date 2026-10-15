"""Create separate maps and assign their native rules without replacing existing assets."""
import unreal

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
stages = (
    ("/Game/Windseed/WindseedGarden", "/Script/PLAYSEED.WindseedGameMode"),
    ("/Game/Playseed/StageSelect", "/Script/PLAYSEED.StageSelectionGameMode"),
    ("/Game/ThunderMeadow/ThunderMeadow", "/Script/PLAYSEED.ThunderMeadowGameMode"),
)
for map_path, mode_path in stages:
    if unreal.EditorAssetLibrary.does_asset_exist(map_path):
        if not levels.load_level(map_path):
            raise RuntimeError(f"Could not load {map_path}")
    elif not levels.new_level(map_path):
        raise RuntimeError(f"Could not create {map_path}")
    mode = unreal.load_class(None, mode_path)
    if not mode:
        raise RuntimeError(f"Missing native class {mode_path}")
    # WorldSettings の GameMode を明示し、どのマップを直接開いても対応するルールにします。
    settings = editor.get_editor_world().get_world_settings()
    settings.set_editor_property("default_game_mode", mode)
    if not levels.save_current_level():
        raise RuntimeError(f"Could not save {map_path}")
unreal.log("PLAYSEED STAGES: each map has its own GameMode")
