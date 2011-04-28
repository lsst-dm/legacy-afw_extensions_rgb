# -*- python -*-
#
# Setup our environment
#
import glob, os.path, re, os
import lsst.SConsUtils as scons

try:
    scons.ConfigureDependentProducts
except AttributeError:
    import lsst.afw.scons.SconsUtils
    scons.ConfigureDependentProducts = lsst.afw.scons.SconsUtils.ConfigureDependentProducts

product = "afw_extensions_rgb"
env = scons.makeEnv(product,
                    r"$HeadURL$",
                    scons.ConfigureDependentProducts(product)
                    )

env.libs[env["eups_product"]] +=  env.getlibs("daf_base daf_data daf_persistence pex_logging pex_exceptions pex_policy security afw ndarray boost tifflib")
#
# Build/install things
#
for d in Split("doc examples lib python/lsst/afw/extensions/rgb src tests"):
    SConscript(os.path.join(d, "SConscript"))

env['IgnoreFiles'] = r"(~$|\.pyc$|^\.svn$|\.o$)"

Alias("install", [
    env.Install(env['prefix'], "doc"),
    env.Install(env['prefix'], "etc"),
    env.Install(env['prefix'], "examples"),
    env.Install(env['prefix'], "lib"),
    env.Install(env['prefix'], "python"),
    env.Install(env['prefix'], "src"),
    env.Install(env['prefix'], "tests"),
    env.InstallEups(os.path.join(env['prefix'], "ups")),
])

scons.CleanTree(r"*~ core *.so *.os *.o *.pyc")
#
# Build TAGS files
#
files = scons.filesToTag()
if files:
    env.Command("TAGS", files, "etags -o $TARGET $SOURCES")

env.Declare()
env.Help("""
LSST FrameWork packages
""")

