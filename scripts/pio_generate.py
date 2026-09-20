# PlatformIO pre-build hook; execute validation with PlatformIO's interpreter.
Import("env")
import subprocess
subprocess.check_call([env.subst("$PYTHONEXE"), "scripts/generate_config.py"])
