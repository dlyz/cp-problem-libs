# $sources = Get-ChildItem -Path "../src" -Recurse
# $vstemplate = Get-ChildItem -Path "../vs" -Recurse
# $files = $sources + $vstemplate

$templateXmlns = @{ns = "http://schemas.microsoft.com/developer/vstemplate/2005"}
$templateNameXml = Select-Xml -Namespace $templateXmlns -LiteralPath "../vs/MyTemplate.vstemplate" -XPath "/ns:VSTemplate/ns:TemplateData/ns:Name"
$templateName = $templateNameXml.node.InnerText

$targetDirPath = "../build"

New-Item $targetDirPath -ItemType directory -Force

Compress-Archive -Path ("../src/*", "../vs/*") -DestinationPath ($targetDirPath + "/" + $templateName + ".zip") -Force