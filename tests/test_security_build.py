from pathlib import Path
import unittest
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
NS = {'m': 'http://schemas.microsoft.com/developer/msbuild/2003'}

class SecurityBuildTests(unittest.TestCase):
    def test_download_code_is_not_reachable(self):
        main = (ROOT / 'LuconiaInjector/Main.cpp').read_text()
        downloader = (ROOT / 'LuconiaInjector/Downloader.h').read_text()
        self.assertNotIn('Downloader::', main)
        self.assertNotIn('URLDownloadToFile', downloader)
        self.assertNotIn('curl_easy_', downloader)
        self.assertNotIn('media.luconia.net', downloader)
        self.assertNotIn('Downloader::filePath', main)
        self.assertIn('GetWindowText(GetDlgItem(hWnd, ID_PATH_EDIT)', main)

    def test_binary_curl_is_not_linked(self):
        tree = ET.parse(ROOT / 'LuconiaInjector/LuconiaInjector.vcxproj')
        for tag in ('AdditionalDependencies', 'LibraryPath', 'IncludePath'):
            for node in tree.findall('.//m:' + tag, NS):
                self.assertNotIn('curl', node.text or '')

if __name__ == '__main__':
    unittest.main()
