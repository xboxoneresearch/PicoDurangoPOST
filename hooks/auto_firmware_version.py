from datetime import datetime
import subprocess

Import("env")

def get_firmware_specifier_build_flag():
    # Either TAG_NAME on clean tree matching the tag
    # Or TAG_NAME-dev if changes since last annotated tag were made
    ret = subprocess.run(["git", "describe", "--dirty=-dev", "--tags", "--abbrev=0"], stdout=subprocess.PIPE, text=True)
    ret.check_returncode()

    build_version = ret.stdout.strip()
    build_flag = "-D __FW_VERSION__=\\\"" + build_version + "\\\""
    print ("Firmware Revision: " + build_version)
    return (build_flag)

def get_build_date_build_flag():
    build_date = datetime.now().strftime("%Y%m%d")
    build_flag = "-D __BUILD_DATE__=" + build_date
    print("Build date: " + build_date)
    return (build_flag)

env.Append(
    BUILD_FLAGS=[
        get_firmware_specifier_build_flag(),
        get_build_date_build_flag()
    ]
)
