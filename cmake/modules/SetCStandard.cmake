#
# Module: SetCStandard
#
# Description:
# Sets the C standard for the target and disables
# vendor (LLVM, GNU, Microsoft, etc.) extensions.
#
# Author:
# マギルゥーベルベット (magiruuvelvet)
#

macro (SetCStandard TargetName CVer)
    set_property(TARGET ${TargetName} PROPERTY C_STANDARD ${CVer})
    set_property(TARGET ${TargetName} PROPERTY C_STANDARD_REQUIRED ON)
    set_property(TARGET ${TargetName} PROPERTY C_EXTENSIONS OFF)
endmacro()
