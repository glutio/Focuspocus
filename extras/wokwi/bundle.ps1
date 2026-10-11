# Makes flat copies of the examples for wokwi.com projects in build/wokwi/<Name>:
# sketch files, diagram.json, libraries.txt (Adafruit libraries) and the
# Focuspocus, Buratino and Eventfun sources, with <...> includes of the
# copied files changed to "...". Upload all files of a folder to a new
# Wokwi project, then put its URL in examples/<Name>/wokwi-project.txt.
#   .\extras\wokwi\bundle.ps1                # all examples
#   .\extras\wokwi\bundle.ps1 Demo Dialog    # some examples
param([Parameter(ValueFromRemainingArguments)][string[]]$Examples)

$root = (Resolve-Path "$PSScriptRoot\..\..").Path
$sources = @("$root\src")
foreach ($name in 'Eventfun', 'Buratino') {
  $dir = @([Environment]::GetEnvironmentVariable("$($name.ToUpper())_DIR"), "$root\..\$name") |
    Where-Object { $_ -and (Test-Path "$_\src") } | Select-Object -First 1
  if (-not $dir) { throw "$name not found: set $($name.ToUpper())_DIR" }
  $sources += (Resolve-Path "$dir\src").Path
}
if (-not $Examples) { $Examples = Get-ChildItem "$root\examples" -Directory | ForEach-Object Name }

# Adafruit header -> Library Manager names
$adafruit = [ordered]@{
  'Adafruit_ILI9341.h' = 'Adafruit GFX Library', 'Adafruit ILI9341'
  'Adafruit_FT6206.h'  = 'Adafruit FT6206 Library'
  'Adafruit_SSD1306.h' = 'Adafruit GFX Library', 'Adafruit SSD1306'
}

foreach ($name in $Examples) {
  $out = "$root\build\wokwi\$name"
  Remove-Item $out -Recurse -Force -ErrorAction SilentlyContinue
  New-Item $out -ItemType Directory -Force | Out-Null
  $sketch = Get-ChildItem "$root\examples\$name\*" -File -Include *.ino, *.h, *.cpp, diagram.json
  $sketch | Copy-Item -Destination $out
  foreach ($dir in $sources) { Get-ChildItem $dir -File | Copy-Item -Destination $out }

  $files = Get-ChildItem "$out\*" -File -Include *.ino, *.h, *.cpp
  $local = $files.Name
  $libs = @()
  foreach ($file in $files) {
    $text = [IO.File]::ReadAllText($file.FullName)
    $new = [regex]::Replace($text, '#include\s*<([^>]+)>', {
        param($m) if ($local -contains $m.Groups[1].Value) { "#include `"$($m.Groups[1].Value)`"" } else { $m.Value } })
    if ($new -ne $text) { [IO.File]::WriteAllText($file.FullName, $new) }
    if ($sketch.Name -contains $file.Name) {
      foreach ($h in $adafruit.Keys) { if ($text -match [regex]::Escape("<$h>")) { $libs += $adafruit[$h] } }
    }
  }
  $libs = @($libs | Select-Object -Unique)
  [IO.File]::WriteAllText("$out\libraries.txt", ($libs -join "`n") + "`n")
  Write-Host "$name -> $out ($($libs -join ', '))"
}
