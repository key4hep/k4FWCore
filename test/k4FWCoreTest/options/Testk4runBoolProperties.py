"""Minimal configuration for testing boolean k4run properties."""

from Configurables import MetadataSvc


# Explicitly set one property; leave the others at their defaults.
MetadataSvc("MetadataSvc").SkipIfSameValue = True
