# Builds each example and runs its Wokwi scenario (examples/<Name>/test.yaml),
# which compares screenshots with examples/<Name>/screenshots/*.png.
#   .\extras\wokwi\run.ps1                  # all examples
#   .\extras\wokwi\run.ps1 Dialog OledMenu  # some examples
#   .\extras\wokwi\run.ps1 -Update          # re-record the reference screenshots
# Needs arduino-cli (arduino:avr core, Adafruit ILI9341/FT6206/SSD1306 libraries),
# wokwi-cli and the WOKWI_CLI_TOKEN environment variable.
param([switch]$Update, [Parameter(ValueFromRemainingArguments)][string[]]$Examples)

$root = Resolve-Path "$PSScriptRoot\..\.."
# Eventfun and Buratino: <NAME>_DIR, a checkout next to this repository, or
# the Arduino libraries folder (where the Library Manager puts them).
$libraries = foreach ($name in 'Eventfun', 'Buratino') {
  $dir = @([Environment]::GetEnvironmentVariable("$($name.ToUpper())_DIR"), "$root\..\$name") |
    Where-Object { $_ -and (Test-Path "$_\src") } | Select-Object -First 1
  if ($dir) { '--library'; (Resolve-Path $dir).Path }
}
if (-not $Examples) {
  $Examples = Get-ChildItem "$root\examples" -Directory |
    Where-Object { Test-Path "$($_.FullName)\test.yaml" } | ForEach-Object Name
}

$failed = @()
foreach ($name in $Examples) {
  $dir = "$root\examples\$name"
  Write-Host "== $name" -ForegroundColor Cyan
  arduino-cli compile -b arduino:avr:mega --library $root @libraries --output-dir "$dir\build" $dir | Out-Null
  if ($LASTEXITCODE) { $failed += "$name (build)"; continue }

  $scenario = 'test.yaml'
  if ($Update) {
    New-Item -ItemType Directory -Force "$dir\screenshots" | Out-Null
    (Get-Content "$dir\test.yaml" -Raw) -replace 'compare-with:', 'save-to:' |
      Set-Content "$dir\update.yaml"
    $scenario = 'update.yaml'
  }
  wokwi-cli --timeout 30000 --scenario $scenario $dir
  if ($LASTEXITCODE) { $failed += $name }
  if ($Update) { Remove-Item "$dir\update.yaml" }
}

if ($failed) { Write-Host "FAILED: $($failed -join ', ')" -ForegroundColor Red; exit 1 }
Write-Host "All passed." -ForegroundColor Green
