#!/usr/bin/env lua

local bigint = require "bigint"
local inspect = require "inspect"
local os = require "os"

local b1 = bigint:new("3")
assert(b1 == bigint:new("3"))
assert(b1 == bigint:new(3))
assert(b1 ~= bigint:new("4"))
assert(b1 ~= bigint:new(4))

local b2 = bigint:new("33")

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
assert(b3 <= 40)
assert(not (40 < b3))
assert(not (b3 < 40))
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
assert(b5:shiftleft(100):shiftright(100) == b5)

local b6 = bigint:new(2)
assert(b6:expmod(100, 50) == bigint:new(26)) -- (2^100)%50 == 26

--       { "inv",          bigint_inv                  },
--       { "gcd",          bigint_gcd                  },


print("All tests passed")
