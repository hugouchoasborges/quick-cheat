param([string]$OutputDirectory=(Join-Path $PSScriptRoot '../build/demo-markdown'))
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
[IO.Directory]::CreateDirectory($OutputDirectory)|Out-Null
function Write-DemoImage($FileName,$Title,$Subtitle,$Panels){
 $bitmap=[Drawing.Bitmap]::new(960,360)
 $graphics=[Drawing.Graphics]::FromImage($bitmap)
 $brushes=@();$fonts=@()
 try{
  $graphics.SmoothingMode=[Drawing.Drawing2D.SmoothingMode]::AntiAlias
  $graphics.TextRenderingHint=[Drawing.Text.TextRenderingHint]::ClearTypeGridFit
  $graphics.Clear([Drawing.ColorTranslator]::FromHtml('#1E1E1E'))
  $foreground=[Drawing.SolidBrush]::new([Drawing.ColorTranslator]::FromHtml('#F4F4F4'))
  $muted=[Drawing.SolidBrush]::new([Drawing.ColorTranslator]::FromHtml('#B4B4B4'))
  $accent=[Drawing.SolidBrush]::new([Drawing.ColorTranslator]::FromHtml('#CEB4FF'))
  $panel=[Drawing.SolidBrush]::new([Drawing.ColorTranslator]::FromHtml('#2D2D2D'))
  $brushes=@($foreground,$muted,$accent,$panel)
  $heading=[Drawing.Font]::new('Segoe UI',24,[Drawing.FontStyle]::Bold)
  $caption=[Drawing.Font]::new('Segoe UI',12)
  $label=[Drawing.Font]::new('Segoe UI',17,[Drawing.FontStyle]::Bold)
  $body=[Drawing.Font]::new('Segoe UI',17)
  $fonts=@($heading,$caption,$label,$body)
  $graphics.DrawString('QUICK CHEAT / DEMO',$caption,$accent,28,18)
  $graphics.DrawString($Title,$heading,$foreground,28,46)
  $graphics.DrawString($Subtitle,$caption,$muted,30,94)
  $width=(904-16*($Panels.Count-1))/$Panels.Count
  for($index=0;$index -lt $Panels.Count;$index++){
   $x=28+$index*($width+16)
   $graphics.FillRectangle($panel,[single]$x,134,[single]$width,170)
   $graphics.FillRectangle($accent,[single]$x,134,4,170)
   $graphics.DrawString($Panels[$index].heading,$label,$accent,[single]($x+18),150)
   $rect=[Drawing.RectangleF]::new([single]($x+18),190,[single]($width-36),100)
   $graphics.DrawString($Panels[$index].body,$body,$foreground,$rect)
  }
  $graphics.DrawString('Illustration for local workflow testing. No message is sent.',$caption,$muted,30,324)
  $bitmap.Save((Join-Path $OutputDirectory $FileName),[Drawing.Imaging.ImageFormat]::Png)
 }finally{foreach($font in $fonts){$font.Dispose()};foreach($brush in $brushes){$brush.Dispose()};$graphics.Dispose();$bitmap.Dispose()}
}
Write-DemoImage 'prompt-guide.png' 'Build a clear prompt' 'GOAL  >  CONTEXT  >  FORMAT' @(@{heading='1. Goal';body='Summarize these notes into a project update.'},@{heading='2. Context';body='Audience: the project team. Keep the facts.'},@{heading='3. Format';body='English. Three short bullet points.'})
Write-DemoImage 'draft-guide.png' 'Improve a draft' 'COMPARE THE ORIGINAL WITH THE REVISION' @(@{heading='Before';body='We did some changes and fixed a few things this week.'},@{heading='After';body='This week: added local workflows and improved shortcut navigation.'})
Write-DemoImage 'keyboard-guide.png' 'Test a shortcut safely' 'USE AN UNSENT SAMPLE IN THE CHAT INPUT' @(@{heading='1. Type';body='Quick Cheat demo'},@{heading='2. Select';body='Ctrl + A'},@{heading='3. Confirm';body='Text is selected. Nothing was sent.'})
Write-Output "Demo illustrations: $([IO.Path]::GetFullPath($OutputDirectory))"
