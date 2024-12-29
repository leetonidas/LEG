
def FeatureBIGENDIAN : SubtargetFeature<"bigendian", "m_isBigEndian", "true", "The device uses instructions encoded as big endian">;


def FeatureInsEncryption : SubtargetFeature<"insencryption", "m_hasInsEncryption", "true", "The device uses instructions stream encryption">;
def HasInsEncryption : Predicate<"Subtarget->hasInsEncryption()">,
                       AssemblerPredicate<(all_of FeatureInsEncryption),
                                          "Plain instruction stream">;
def HasNotInsEncryption : Predicate<"!Subtarget->hasInsEncryption()">,
						  AssemblerPredicate<(any_of (not FeatureInsEncryption)),
						                     "Instruction stream encryption">;


def : Processor<"genbig", NoItineraries, [FeatureBIGENDIAN]>;