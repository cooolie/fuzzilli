// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
import XCTest
@testable import Fuzzilli

extension BaseInstructionMutator {
    func mockMutate(_ program: Program, for fuzzer: Fuzzer, at index: Int) -> Program {
        beginMutation(of: program)
        let b = fuzzer.makeBuilder()
        b.adopting(from: program) {
            for instr in program {
                if instr.index == index {
                    mutate(instr, b)
                } else {
                    b.adopt(instr, keepTypes: true)
                }
            }
        }

        return b.finalize()
    }
}

class MutationsTests: XCTestCase {

    func testPrepareMutationRuntimeTypes() {
        let fuzzer = makeMockFuzzer()
        var b = fuzzer.makeBuilder()
        b.phi(b.loadInt(47))
        fuzzer.engine.setPrefix(b.finalize())
        b = fuzzer.makeBuilder()
        let x = b.phi(b.loadInt(42))
        b.beginIf(b.loadBool(true)) {
            b.copy(b.loadFloat(1.1), to: x)
        }
        b.beginElse() {
            b.copy(b.loadString("test"), to: x)
        }
        b.endIf()
        let program = b.finalize()
        let types: [Type] = [.integer, .number, .boolean, .float]
        program.runtimeTypes = VariableMap<Type>(types)

        let preparedProgram = fuzzer.engine.prepareForMutation(program)
        XCTAssertEqual(preparedProgram.runtimeTypes, VariableMap<Type>([nil, nil] + types))
    }

    func testInputMutatorRuntimeTypes() {
        let fuzzer = makeMockFuzzer()
        let b = fuzzer.makeBuilder()
        b.loadString("test")
        let x = b.binary(b.loadInt(1), b.loadInt(2), with: .Add)
        b.phi(x)
        let program = b.finalize()
        var types: [Type?] = [.string, .integer, .integer, .integer, .integer]
        program.runtimeTypes = VariableMap<Type>(types)

        // Mutate only 3rd instruction
        let mutatedProgram = InputMutator().mockMutate(program, for: fuzzer, at: 3)

        // v3 was mutated, we should discard this type
        // v4 depends on mutated v3, but for now we keep its type
        types[3] = nil
        XCTAssertEqual(mutatedProgram.runtimeTypes, VariableMap<Type>(types))
    }

    func testOperationMutatorRuntimeTypes() {
        let fuzzer = makeMockFuzzer()
        let b = fuzzer.makeBuilder()
        b.loadString("test")
        b.binary(b.loadInt(1), b.loadInt(2), with: .Add)
        let program = b.finalize()
        var types: [Type?] = [.string, .integer, .integer, .integer]
        program.runtimeTypes = VariableMap<Type>(types)

        let mutatedIntProgram = OperationMutator().mockMutate(program, for: fuzzer, at: 1)

        // Only int was mutated, so we keep all types
        XCTAssertEqual(mutatedIntProgram.runtimeTypes, VariableMap<Type>(types))

        let mutatedBinaryOpProgram = OperationMutator().mockMutate(program, for: fuzzer, at: 3)

        // v3 is result of mutated binary operation, so we should discard this type
        types.removeLast()
        XCTAssertEqual(mutatedBinaryOpProgram.runtimeTypes, VariableMap<Type>(types))
    }
}

extension MutationsTests {
    static var allTests : [(String, (MutationsTests) -> () throws -> Void)] {
        return [
            ("testPrepareMutationRuntimeTypes", testPrepareMutationRuntimeTypes),
            ("testInputMutatorRuntimeTypes", testInputMutatorRuntimeTypes),
            ("testOperationMutatorRuntimeTypes", testOperationMutatorRuntimeTypes),
        ]
    }
}
