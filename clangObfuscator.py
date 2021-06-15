import os
import json
import subprocess
import urllib.request
import zipfile
import shutil
import re


def download_callback(block_num, block_size, total_size):
    all_size = total_size / block_size
    ten_percent = all_size / 10
    ten_percent_int = int(ten_percent)
    if block_num % ten_percent_int == 0:
        percent = int(block_num / ten_percent_int) * 10
        print('download %d%%' % percent)


def download_file(file_path, file_name):
    urllib.request.urlretrieve(file_path, file_name, download_callback)


def unzip_file(file_path, target_path):
    file = zipfile.ZipFile(file_path, 'r')
    file.extractall(target_path)


def get_sdk_version():
    all_lines = subprocess.getoutput('xcodebuild -showsdks')
    all_lines = all_lines.splitlines()
    rule = r'.*?(-sdk iphonesimulator)([\d\.]+)'
    for line in all_lines:
        match_obj = re.match(rule, line)
        if match_obj:
            return match_obj.group(2)
    return ''


def do(version, tool_url, run_test):
    if not os.path.exists('./clangObfuscator/'):
        os.mkdir('./clangObfuscator/')

    tool_file_path = './clangObfuscator/%s.zip' % version
    unzip_file_path = './clangObfuscator/%s' % version

    if not os.path.exists(unzip_file_path):
        if os.path.exists(tool_file_path):
            unzip_file(tool_file_path, unzip_file_path)
        else:
            download_file(tool_url, tool_file_path)
            unzip_file(tool_file_path, unzip_file_path)

    ori_cwd = os.getcwd()
    os.chdir(unzip_file_path + '/bin/')
    subprocess.getoutput('chmod a+x ./clang')
    subprocess.getoutput('chmod a+x ./CodeStyleRefactor')
    os.chdir(ori_cwd)

    sdk_version = get_sdk_version()
    print('\n')
    print('sdk_version: ' + sdk_version)

    if run_test:
        os.chdir(unzip_file_path + '/ATClangObfuscatorTest/')
        xcodebuild_cmd = ('xcodebuild clean build -project ATClangObfuscatorTest.xcodeproj -scheme ATClangObfuscatorTest ' \
                         '-configuration Analyze -arch x86_64 -sdk iphonesimulator%s  | tee xcodebuild.log | xcpretty ' \
                         '--report json-compilation-database --output compile_commands.json') % sdk_version
        print('\n')
        print(xcodebuild_cmd)
        print('\n')
        print(subprocess.getoutput(xcodebuild_cmd))
        os.chdir('../bin/')
        obfuscator_cmd = './CodeStyleRefactor ../ATClangObfuscatorTest/'
        print('\n')
        print(obfuscator_cmd)
        print('\n')
        print(subprocess.getoutput(obfuscator_cmd))
    else:
        print('do it yourself :)')


if __name__ == '__main__':
    do('1.0.2', 'https://github.com/linzhiman/ATClangObfuscator/releases/download/1.0.2/ATClangObfuscator.zip', True)
