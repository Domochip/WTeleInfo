import os
import shutil

sourceFolder = r'..\WirelessBase\src\base'
destinationFolder = r'.\src\base'

if os.path.exists(sourceFolder) and os.path.abspath(sourceFolder) != os.path.abspath(destinationFolder):
    shutil.rmtree(destinationFolder,True)
    shutil.copytree(sourceFolder,destinationFolder)