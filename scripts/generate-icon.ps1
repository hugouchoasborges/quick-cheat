$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
$assetRoot=Join-Path (Split-Path $PSScriptRoot -Parent) 'assets'
New-Item -ItemType Directory -Path $assetRoot -Force | Out-Null
# Authored geometry: a lavender keyboard key with a geometric Q, on charcoal.
$svg=@'
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256">
 <title>Quick Cheat</title>
 <rect x="8" y="8" width="240" height="240" rx="52" fill="#1e1e1e"/>
 <rect x="40" y="46" width="176" height="174" rx="30" fill="#8961b8"/>
 <rect x="40" y="36" width="176" height="170" rx="30" fill="#ceb4ff"/>
 <circle cx="125" cy="115" r="40" fill="none" stroke="#292131" stroke-width="18"/>
 <path d="M146 139L168 161" fill="none" stroke="#292131" stroke-width="18" stroke-linecap="round"/>
</svg>
'@
[IO.File]::WriteAllText((Join-Path $assetRoot 'quick-cheat.svg'),$svg,[Text.UTF8Encoding]::new($false))
function Rounded-Path([single]$x,[single]$y,[single]$width,[single]$height,[single]$radius){
 $path=[Drawing.Drawing2D.GraphicsPath]::new();$diameter=2*$radius
 $path.AddArc($x,$y,$diameter,$diameter,180,90);$path.AddArc($x+$width-$diameter,$y,$diameter,$diameter,270,90)
 $path.AddArc($x+$width-$diameter,$y+$height-$diameter,$diameter,$diameter,0,90);$path.AddArc($x,$y+$height-$diameter,$diameter,$diameter,90,90);$path.CloseFigure();return ,$path
}
$frames=[Collections.Generic.List[byte[]]]::new();$sizes=@(16,20,24,32,40,48,64,96,128,256)
foreach($size in $sizes){
 $renderSize=$size*4;$bitmap=[Drawing.Bitmap]::new($renderSize,$renderSize,[Drawing.Imaging.PixelFormat]::Format32bppArgb)
 $graphics=[Drawing.Graphics]::FromImage($bitmap);$graphics.SmoothingMode='AntiAlias';$graphics.ScaleTransform($renderSize/256.0,$renderSize/256.0)
 try{
  foreach($shape in @(@(8,8,240,240,52,'#1e1e1e'),@(40,46,176,174,30,'#8961b8'),@(40,36,176,170,30,'#ceb4ff'))){
   $path=Rounded-Path $shape[0] $shape[1] $shape[2] $shape[3] $shape[4];$brush=[Drawing.SolidBrush]::new([Drawing.ColorTranslator]::FromHtml($shape[5]))
   try{$graphics.FillPath($brush,$path)}finally{$brush.Dispose();$path.Dispose()}
  }
  $pen=[Drawing.Pen]::new([Drawing.ColorTranslator]::FromHtml('#292131'),18);$pen.StartCap='Round';$pen.EndCap='Round'
  try{$graphics.DrawEllipse($pen,85,75,80,80);$graphics.DrawLine($pen,146,139,168,161)}finally{$pen.Dispose()}
 }finally{$graphics.Dispose()}
 $small=[Drawing.Bitmap]::new($size,$size,[Drawing.Imaging.PixelFormat]::Format32bppArgb);$scaler=[Drawing.Graphics]::FromImage($small);$scaler.InterpolationMode='HighQualityBicubic';$scaler.CompositingMode='SourceCopy'
 try{$scaler.DrawImage($bitmap,0,0,$size,$size)}finally{$scaler.Dispose();$bitmap.Dispose()}
 $stream=[IO.MemoryStream]::new()
 try{$small.Save($stream,[Drawing.Imaging.ImageFormat]::Png);$frames.Add($stream.ToArray());if($size-eq 256){$small.Save((Join-Path $assetRoot 'quick-cheat.png'),[Drawing.Imaging.ImageFormat]::Png)}}finally{$stream.Dispose();$small.Dispose()}
}
$output=[IO.File]::Create((Join-Path $assetRoot 'quick-cheat.ico'));$writer=[IO.BinaryWriter]::new($output)
try{
 $writer.Write([uint16]0);$writer.Write([uint16]1);$writer.Write([uint16]$sizes.Count);$offset=6+16*$sizes.Count
 for($i=0;$i-lt $sizes.Count;$i++){$dimension=if($sizes[$i]-eq 256){0}else{$sizes[$i]};$writer.Write([byte]$dimension);$writer.Write([byte]$dimension);$writer.Write([byte]0);$writer.Write([byte]0);$writer.Write([uint16]1);$writer.Write([uint16]32);$writer.Write([uint32]$frames[$i].Length);$writer.Write([uint32]$offset);$offset+=$frames[$i].Length}
 foreach($frame in $frames){$writer.Write([byte[]]$frame)}
}finally{$writer.Dispose();$output.Dispose()}
Write-Output "Icon assets generated in $assetRoot"
