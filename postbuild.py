import sys
import os

if(len(sys.argv) < 2):
    print("No arguments were provided for the post build script")

args = sys.argv

root_dir = args[1]
root_res_dir = root_dir + "/res"
build_dir = args[2]
build_res_dir = build_dir + "/res"

if(os.path.exists(build_res_dir)):
    quit()

print("The root dir is: ", root_res_dir)
print("The build dir is: ", build_res_dir)

print("The relative path of root dir relative to build dir is: ",
      os.path.relpath(root_res_dir, build_res_dir))


def symlink_rel(src: str, dst: str):
    rel_path_src = os.path.relpath(src, os.path.dirname(dst))
    os.symlink(rel_path_src, dst, True)


symlink_rel(root_res_dir, build_res_dir)
# os.symlink("res", "build/res", True)
