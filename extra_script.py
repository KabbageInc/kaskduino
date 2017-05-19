Import("env")

import time, os

def before_upload(source, target, env):
  print "before_upload: resetting GPIO18 for Alamode"
  os.system("sudo gpio export 18 out")
  os.system("sudo gpio write 18 0")
  time.sleep(0.1)
  os.system("sudo gpio write 18 1")
  os.system("sudo gpio unexport 18")

env.AddPreAction("upload", before_upload)