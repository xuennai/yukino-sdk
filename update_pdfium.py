import urllib.request
import json
import tarfile
import os
import shutil
import subprocess
import sys

# ==========================================
# 配置路径 (基于项目根目录)
# ==========================================
MODULE_NAME = "yukino-sdk"
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
MODULE_DIR = os.path.join(BASE_DIR, MODULE_NAME)

JNI_DIR = os.path.join(MODULE_DIR, "src", "main", "jniLibs", "arm64-v8a")
INCLUDE_DIR = os.path.join(MODULE_DIR, "src", "main", "cpp", "pdfium", "include")
VERSION_FILE = os.path.join(MODULE_DIR, "pdfium_version.txt") # 用于记录当前版本

TEMP_TGZ = os.path.join(BASE_DIR, "pdfium_download.tgz")
TEMP_EXTRACT = os.path.join(BASE_DIR, "temp_extract")

API_URL = "https://api.github.com/repos/bblanchon/pdfium-binaries/releases/latest"

def run_gradle_build():
    print(f"🛠️ 正在启动 Gradle 编译 {MODULE_NAME}...")
    gradle_cmd = "gradlew.bat" if os.name == "nt" else "./gradlew"
    gradle_path = os.path.join(BASE_DIR, gradle_cmd)

    try:
        subprocess.run([gradle_path, f":{MODULE_NAME}:assembleRelease"], check=True)
        print("🎉 编译成功！")
        aar_path = os.path.join(MODULE_DIR, "build", "outputs", "aar", f"{MODULE_NAME}-release.aar")
        if os.path.exists(aar_path):
            print(f"📍 AAR 产物位置: {aar_path}")
    except subprocess.CalledProcessError:
        print("❌ 编译过程中出错，请检查控制台 Gradle 输出信息。")

def safe_extract(tar, member, path):
    """兼容高版本 Python 的安全提取策略，消除 DeprecationWarning"""
    if sys.version_info >= (3, 12):
        tar.extract(member, path=path, filter='data')
    else:
        tar.extract(member, path=path)

def main():
    print("🚀 [1/5] 正在检查 GitHub 上 PDFium 的最新版本...")
    req = urllib.request.Request(API_URL, headers={'User-Agent': 'Mozilla/5.0'})

    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            tag_name = data['tag_name']

            # --- 版本比对逻辑 ---
            if os.path.exists(VERSION_FILE):
                with open(VERSION_FILE, 'r') as f:
                    current_version = f.read().strip()
                if current_version == tag_name:
                    print(f"✨ 当前已是最新版本 ({tag_name})，文件一致，跳过下载！")
                    # 依然触发编译以防你改了本地的 C++ 代码
                    run_gradle_build()
                    return

            print(f"✨ 发现新版本: {tag_name}")

            download_url = next(
                (asset['browser_download_url'] for asset in data['assets']
                 if asset['name'] == 'pdfium-android-arm64.tgz'),
                None
            )

            if not download_url:
                print("❌ 错误：未在该版本中找到 pdfium-android-arm64.tgz")
                return
    except Exception as e:
        print(f"❌ 网络请求失败: {e}")
        return

    print(f"⬇️ [2/5] 正在下载二进制包...\n🔗 {download_url}")
    urllib.request.urlretrieve(download_url, TEMP_TGZ)

    print("📦 [3/5] 正在精准提取文件并替换旧版本...")
    if os.path.exists(INCLUDE_DIR): shutil.rmtree(INCLUDE_DIR)
    os.makedirs(JNI_DIR, exist_ok=True)
    if os.path.exists(TEMP_EXTRACT): shutil.rmtree(TEMP_EXTRACT)

    with tarfile.open(TEMP_TGZ, "r:gz") as tar:
        for member in tar.getmembers():
            if member.name == "lib/libpdfium.so":
                member.name = os.path.basename(member.name)
                safe_extract(tar, member, path=JNI_DIR)
            elif member.name.startswith("include/"):
                safe_extract(tar, member, path=TEMP_EXTRACT)

    extracted_include = os.path.join(TEMP_EXTRACT, "include")
    if os.path.exists(extracted_include):
        shutil.move(extracted_include, INCLUDE_DIR)

    if os.path.exists(TEMP_EXTRACT): shutil.rmtree(TEMP_EXTRACT)
    if os.path.exists(TEMP_TGZ): os.remove(TEMP_TGZ)

    # 下载并替换成功后，记录最新版本号
    with open(VERSION_FILE, 'w') as f:
        f.write(tag_name)

    print("✅ 文件更新完成！")
    run_gradle_build()

if __name__ == "__main__":
    main()
