"""Generate PLAYSEED's shared palette and sky materials.

Run with UnrealEditor-Cmd, -run=pythonscript -script=<this file>.
No downloaded assets are required. Existing assets are preserved. No course map is generated.
"""
import unreal

tools = unreal.AssetToolsHelpers.get_asset_tools()
material_path = "/Game/Windseed/M_Palette"
if not unreal.EditorAssetLibrary.does_asset_exist(material_path):
    material = tools.create_asset("M_Palette", "/Game/Windseed", unreal.Material, unreal.MaterialFactoryNew())
    tint = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -300, 0)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(0.3, 0.65, 0.4, 1))
    unreal.MaterialEditingLibrary.connect_material_property(tint, "", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 160)
    roughness.set_editor_property("r", 0.8)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

sky_path = "/Game/Windseed/M_Sky"
if not unreal.EditorAssetLibrary.does_asset_exist(sky_path):
    sky = tools.create_asset("M_Sky", "/Game/Windseed", unreal.Material, unreal.MaterialFactoryNew())
    sky.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    sky.set_editor_property("two_sided", True)
    sky.set_editor_property("is_sky", True)
    color = unreal.MaterialEditingLibrary.create_material_expression(sky, unreal.MaterialExpressionConstant3Vector, -300, 0)
    color.set_editor_property("constant", unreal.LinearColor(0.3, 0.59, 0.76, 1))
    unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(sky)
    unreal.EditorAssetLibrary.save_loaded_asset(sky)


unreal.log("PLAYSEED SHARED ASSETS: palette and sky are ready")
