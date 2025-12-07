$path = "src/main.cpp"
$content = Get-Content $path -Raw
$search = "DFSAlgorithm genAlgo(grid, static_cast<unsigned int>(std::time(nullptr)));"
$replace = "DFSAlgorithm genAlgo(grid);"

if ($content.Contains($search)) {
    $content = $content.Replace($search, $replace)
    Set-Content $path $content -NoNewline
    Write-Host "Replaced successfully"
}
else {
    Write-Host "Search string not found"
    exit 1
}
