"""Import only our four changed surface textures. Existing material graphs stay intact."""
from pathlib import Path
import unreal
root=Path(__file__).parent/'ReferenceTextures'
tools=unreal.AssetToolsHelpers.get_asset_tools()
for name in ['Grass','Cave','Sky','Wood']:
    task=unreal.AssetImportTask();task.set_editor_property('filename',str(root/(name+'.tga')))
    task.set_editor_property('destination_path','/Game/Battlefield');task.set_editor_property('destination_name','T_'+name)
    task.set_editor_property('automated',True);task.set_editor_property('save',True);task.set_editor_property('replace_existing',True)
    tools.import_asset_tasks([task]);texture=unreal.load_asset('/Game/Battlefield/T_'+name);assert texture,name
    texture.set_editor_property('filter',unreal.TextureFilter.TF_BILINEAR)
    texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_DEFAULT)
    texture.set_editor_property('srgb',True);unreal.EditorAssetLibrary.save_loaded_asset(texture)
    assert unreal.load_asset('/Game/Battlefield/M_'+name),name
unreal.log('REFERENCE SURFACES PASS: four original textures, explicit color/sRGB; existing materials preserved')
