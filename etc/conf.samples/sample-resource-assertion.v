#==============================================================================
#
# File:         /var/vermont/conf.d/sample-resource-assertion
#
# Language:     Vermont declarative language
#
# Purpose:      A sample showing how resources and assertions work.
#
# Author:       Joe Honton © 2011
#
# Initial date: February 3, 2011
#
#==============================================================================

[sample29:sample29]

[E]
type = resource
request = memory
#runtime_values = show					# vmtModule keyword candidate.  {show|hide} (why not just enable verbose_base_objects = true, etc.
pms = physical_memory_size
pmu = physical_memory_used
pmf = physical_memory_free
pmp = physical_memory_percent
vms = virtual_memory_size
vmu = virtual_memory_used
vmf = virtual_memory_free 
vmp = virtual_memory_percent 
#pass = ObjectE: $pms $pmu $pmf $pmp $vms $vmu $vmf $vmp
							 
#[F]
#type = assertion
#assert = $E.pms LT 1.5*1024*1024		# 1.5 Gb
#assert = $E.vmp LT 70					# 70%

[G]
type = module
#pass = ObjectG: $E.pms $E.pmu $E.pmf $E.pmp $E.vms $E.vmu $E.vmf $E.vmp
assert = $E.vmp LT 70					# 70%

[sample30:H]
type = module
#pass = ObjectH: $sample29:E.pms $sample29:E.pmu $sample29:E.pmf $sample29:E.pmp $sample29:E.vms $sample29:E.vmu $sample29:E.vmf $sample29:E.vmp

[sample31:sample31]

[A]
type = resource
request = cpu
one = average_load_1
five = average_load_5
fifteen = average_load_15

[B]
type = module
#pass = ObjectB: $A.one $A.five $A.fifteen



