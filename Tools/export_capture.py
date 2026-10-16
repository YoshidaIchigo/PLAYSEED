"""Export the complete Unreal AVI capture with continuous, zero-based timestamps.

Requires ffmpeg/ffprobe from a trusted local distribution. Original AVI is read-only.
"""
import argparse,hashlib,json,struct,subprocess
from pathlib import Path

def run(arguments):
    result=subprocess.run([str(x) for x in arguments],check=True,capture_output=True,text=True,encoding='utf-8',errors='replace')
    return result.stdout

def export(source,destination,review,ffmpeg):
    source,destination,review,ffmpeg=map(Path,(source,destination,review,ffmpeg))
    if source.suffix.lower()!='.avi' or destination.suffix.lower()!='.mp4':raise ValueError('Expected AVI input and MP4 output')
    if destination.exists():raise ValueError('Preserve existing video')
    ffprobe=ffmpeg.with_name('ffprobe.exe')
    if not ffmpeg.is_file() or not ffprobe.is_file():raise ValueError('Provide trusted ffmpeg.exe and ffprobe.exe')
    data=source.read_bytes();original_hash=hashlib.sha256(data).hexdigest()
    if data[:4]!=b'RIFF' or data[8:12]!=b'AVI ':raise ValueError('Not an AVI capture')
    header=struct.unpack_from('<14I',data,data.index(b'avih')+8)
    interval,count,width,height=header[0],header[4],header[8],header[9]
    del data
    destination.parent.mkdir(parents=True,exist_ok=True);review.mkdir(parents=True,exist_ok=True)
    run([ffmpeg,'-hide_banner','-loglevel','error','-n','-i',source,'-map','0:v:0','-c:v','libx264','-threads','4','-preset','medium','-crf','20','-pix_fmt','yuv420p','-fps_mode','passthrough','-movflags','+faststart','-an',destination])
    stream=json.loads(run([ffprobe,'-v','error','-count_frames','-select_streams','v:0','-show_entries','stream=width,height,codec_name,nb_read_frames,duration','-of','json',destination]))['streams'][0]
    if (stream['width'],stream['height'],int(stream['nb_read_frames']))!=(width,height,count):raise RuntimeError('Export dropped frames or changed dimensions')
    times=json.loads(run([ffprobe,'-v','error','-select_streams','v:0','-show_frames','-show_entries','frame=best_effort_timestamp_time','-of','json',destination]))['frames']
    timestamps=[float(x['best_effort_timestamp_time']) for x in times]
    expected_interval=interval/1e6
    if len(timestamps)!=count or any(abs(t-i*expected_interval)>.002 for i,t in enumerate(timestamps)):raise RuntimeError('Export timestamps are not continuous from zero')
    duration=float(stream['duration'])
    for name,second in [('VideoStart',0),('VideoEntry',min(1.7,duration*.15)),('VideoEnd',max(0,duration-1))]:
        run([ffmpeg,'-hide_banner','-loglevel','error','-n','-ss',second,'-i',destination,'-frames:v','1',review/(name+'.png')])
    if hashlib.sha256(source.read_bytes()).hexdigest()!=original_hash:raise RuntimeError('Original capture changed')
    result=dict(source_sha256=original_hash,original_frames=count,exported_frames=count,width=width,height=height,duration_seconds=duration,continuous_timestamps=True,export_sha256=hashlib.sha256(destination.read_bytes()).hexdigest(),bytes=destination.stat().st_size,encoder=run([ffmpeg,'-version']).splitlines()[0])
    (review/'ExportVerification.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(result))

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('source');parser.add_argument('destination');parser.add_argument('review');parser.add_argument('--ffmpeg',required=True)
    args=parser.parse_args();export(args.source,args.destination,args.review,args.ffmpeg)
