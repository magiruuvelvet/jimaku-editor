**contains modifications**

pay attentions to them when updating from upstream

- modified line 2071 (now 2074) to be more i18n friendly\
  `result += toLocalString(" " + g + ":\n");`

- use an ordered map structure for help options
  to preserve their insertion order to have control
  where options appear instead of being random

  `#include "ordered_map.hpp"`\
  `#include "ordered_set.hpp"`\
  `tsl::ordered_map<std::string, HelpGroupDetails> m_help;`
