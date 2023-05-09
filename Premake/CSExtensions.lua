require('vstudio')
local cs2005 = premake.vstudio.cs2005
local dotnetbase = premake.vstudio.dotnetbase

-- This should (or at least I hope it does) register 'propertytags' as a field of the 'prj' parameter which is passed down the line
-- Note that I did not know how to specify a list of paired values... this will become apparent down the line, so for now please
-- accept a list of strings...
premake.api.register {
    name = "propertytags",
    scope = "project",
    kind = "list:string",
}

-- The function which is actually adding the XML fields into 'PropertyGroup'
local function insertProperties(prj)
    local elementcount = #prj.propertytags
    if (elementcount % 2) == 0 and elementcount > 0 then
        for i=1, elementcount, 2 do
            local property = prj.propertytags[i]
            local value = prj.propertytags[i + 1]
            _p(2, '<' .. property .. '>' .. value .. '</' .. property .. '>')
        end
    end
end

-- An override of a function inside the dotnetbase namespace which is responsible of creating the group
premake.override(dotnetbase, "projectProperties", function(base, prj)
    _p(1,'<PropertyGroup>')
    local cfg = premake.project.getfirstconfig(prj)
    premake.callArray(dotnetbase.elements.projectProperties, cfg)
    -- Function goes at the end for style reasons... could go into callArray if introduced.
    insertProperties(prj)
    _p(1,'</PropertyGroup>')
end)