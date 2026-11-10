/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 licensing
  vendor:             JuceStarter
  version:            1.0.0
  name:               Licensing
  description:        Derives this machine's code and verifies a signed licence file, offline.
  dependencies:       juce_core
  minimumCppStandard: 20

 END_JUCE_MODULE_DECLARATION
*******************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include <juce_core/juce_core.h>

#include "TrustedKeys.h"
#include "Base64Url.h"
#include "MachineCode.h"
#include "LicenceVerifier.h"
#include "LicenceFile.h"
