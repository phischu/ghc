# -----------------------------------------------------------------------------
#
# (c) 2009 The University of Glasgow
#
# This file is part of the GHC build system.
#
# To understand how the build system works and how to modify it, see
#      http://hackage.haskell.org/trac/ghc/wiki/Building/Architecture
#      http://hackage.haskell.org/trac/ghc/wiki/Building/Modifying
#
# -----------------------------------------------------------------------------

utils/unlit_dist_C_SRCS  = unlit.c
utils/unlit_dist_PROG    = $(GHC_UNLIT_PGM)
utils/unlit_dist_LIBEXEC = YES
utils/unlit_dist_INSTALL = YES

ifneq "$(BINDIST)" "YES"
$(eval $(call build-prog,utils/unlit,dist,0))
endif
