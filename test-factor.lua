#!/usr/bin/env lua

require "std.strict"

local bigint = require "bigint"
local factor = require "bigint.factor"

-- MAIN --
local factors = factor.compute(arg[1])
local count = 0
for _,v in ipairs(factors) do
   if (count == 0) then
      io.write("= ")
   else
      io.write(" * ")
   end
   io.write(v:tostring())
   count = count + 1
end
io.write("\n")
