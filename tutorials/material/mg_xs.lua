--[[ @doc
# Multigroup Cross Sections

## Background

OpenSn is not provided with cross-section libraries. Users are expected to supply their own multigroup cross-section data.
One may use other open-source software to generate this data (e.g., NJOY, Dragon, OpenMC).

The cross sections are read from a plain text file. The ```OPENSN_XSFILE``` format  of that file is as follows:
```
# Add descriptive comments as appropriate
NUM_GROUPS ng
NUM_MOMENTS nmom

SIGMA_T_BEGIN
0 value
.
.
ng-1 value
SIGMA_T_END

SIGMA_A_BEGIN
0 value
.
.
ng-1 value
SIGMA_A_END

TRANSFER_MOMENTS_BEGIN
# Add descriptive comments as appropriate
M_GPRIME_G_VAL 0 0 0 value
.
M_GPRIME_G_VAL moment gprime g value
.
M_GPRIME_G_VAL nmom-1 ng-1 ng-1 value
TRANSFER_MOMENTS_END

```

## Materials

We create a material and add a property to it:
+ TRANSPORT_XSECTIONS for the transport cross sections

Recall that lua indexing starts at 1.
--]]
-- Add materials
materials = {}
materials[1] = PhysicsAddMaterial("Material_A");

PhysicsMaterialAddProperty(materials[1],TRANSPORT_XSECTIONS)
--[[ @doc

## Cross Sections

We assign the cross sections to the material by loading the file containing the cross sections.
--]]
PhysicsMaterialSetProperty(materials[1],TRANSPORT_XSECTIONS,
        OPENSN_XSFILE,"xs_1g_MatA.xs")
