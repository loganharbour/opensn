--############################################### Create cross sections
xs = {}

for m=0,1 do
    xs[tostring(m)] = PhysicsTransportXSCreate()
end

PhysicsTransportXSSet(xs["0"],OPENSN_XSFILE,"xs_water_g2.xs")
PhysicsTransportXSSet(xs["1"],OPENSN_XSFILE,"xs_fuel_g2.xs")

water_xs = PhysicsTransportXSGet(xs["0"])
num_groups = water_xs["num_groups"]

--############################################### Create materials
materials = {}
for m=0,1 do
    key = tostring(m)
    materials[key] = PhysicsAddMaterial("Material_"..key)
    PhysicsMaterialAddProperty(key,TRANSPORT_XSECTIONS)
    PhysicsMaterialSetProperty(key,TRANSPORT_XSECTIONS, EXISTING,xs[key])
end
