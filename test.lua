#!/usr/bin/env lua

require "std.strict"

local bigint = require "bigint"
local factor = require "bigint.factor"
local inspect = require "inspect"
local os = require "os"

-- return true if the elements in 'a' match the elements in 'b'
function arrayMatch(a, b)
   if (#a ~= #b) then
      return false
   end

   local i
   for i=1, #a do
      if (type(b[i]) == "number" or type(b[i]) == "string") then
	 b[i] = bigint:new(b[i])
      end
      if (a[i] ~= b[i]) then
	 return false
      end
   end

   return true
end

local b1 = bigint:new("3")
assert(b1 == bigint:new("3"))
assert(b1 == bigint:new(3))
assert(b1 ~= bigint:new("4"))
assert(b1 ~= bigint:new(4))
assert(b1:tonumber() == 3)

local b2 = bigint:new("33")
assert(b2:tonumber() == 33)

local b3 = b1 + b2
assert(b3 == bigint:new(36))

b1 = bigint:new("4")
b3 = b3 + b1
assert(b3 == bigint:new(40))
assert(b3 / b1 * 10 == bigint:new(100))
assert(b3 % 3 == bigint:new(1))

local b4 = b3 ^ 4
assert(b4 == bigint:new(2560000))

assert(-b3 == bigint:new(-40))
assert(-b3 == bigint:new("-40"))

assert(b3 ~= b1)
assert(b3 == b3)
local b4 = b3 + 3 - 2 - 1
assert(b4 == b3)
assert(b3 == b4)

assert(not (b3==40)) -- fails b/c you can't compare a userdata with a number :(

assert(b3 == bigint:new(40))
assert(b3:tonumber() <= 40)
assert(not (40 < b3:tonumber()))
assert(not (b3:tonumber() < 40))
assert(not (bigint:new(40) < b3))
assert(not (b3 < bigint:new(40)))

-- shift left/right by 1 uses a different internal algorithm than 2+, so do both
assert(b3:shiftleft(1)  == bigint:new(80))
assert(b3:shiftright(1) == bigint:new(20))
assert(b3:shiftleft(2)  == bigint:new(160))
assert(b3:shiftright(2) == bigint:new(10))

-- Test bit-shifts around word boundaries
local b5 = bigint:new(1)

assert(b5:shiftleft(15) == bigint:new(32768))
assert(b5:shiftleft(16) == bigint:new(65536))
assert(b5:shiftleft(17) == bigint:new(131072))

assert(b5:shiftleft(31) == bigint:new("2147483648"))
assert(b5:shiftleft(32) == bigint:new("4294967296"))
assert(b5:shiftleft(33) == bigint:new("8589934592"))

-- and now we're talking about very large numbers that require string initialization...
assert(b5:shiftleft(64) == bigint:new("18446744073709551616"))
assert(b5:shiftleft(65) == bigint:new("36893488147419103232"))
assert(b5:shiftleft(100) == bigint:new("1267650600228229401496703205376"))
local b7 = b5
assert(b5:shiftleft(100):shiftright(101) == b7:shiftright(1))

local b6 = bigint:new(2)
assert(b6:expmod(100, 50) == bigint:new(26)) -- (2^100)%50 == 26

--       { "inv",          bigint_inv                  },

assert(bigint.gcd(10,20):tostring() == "10")
assert(bigint.gcd(45,330):tostring() == "15")
assert(bigint.gcd(45,"330"):tostring() == "15")
assert(bigint.gcd(258258,48135981) == bigint:new(3))
assert(bigint.gcd(258258,48135981):tostring() == "3")

assert(arrayMatch(factor.compute(2), { 2 } ))
assert(arrayMatch(factor.compute(4), { 2, 2 } ))
assert(arrayMatch(factor.compute(bigint:new(2)), { 2 } ))
assert(arrayMatch(factor.compute("2"), { 2 } ))
assert(arrayMatch(factor.compute("99999999999999999999"), { 3, 3, 11, 41, 101, 271, 3541, 9091, 27961 } ))

print("All tests passed")
