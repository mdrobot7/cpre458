import re
import os
import sys
import subprocess
import bisect
import operator
import array

"""
Parse debugging information and assembly to determine stack usage of a program.
Pass arguments: python3 stack_analyze.py program.elf program.stack

Log output will be written to program.stack

This requires a little help from the linker.
The linker must provide the following symbols:
ROM_LENGTH = Total length of available ROM storage (total FLASH)
RAM_LENGTH = Total length of available RAM storage (total SRAM)
_srom, _erom = Start and end of used region of ROM storage. This should include relocate data. (used FLASH)
_sram, _eram = Start and end of used region of RAM storage. This should include relocate data and stack space. (used SRAM)
_sstack, _estack = Start and end of stack region. This is subtracted from SRAM values above.
exception_table = Symbol (with defined size) indicating the location and length of the exception table in memory.
  Must reside in either the start of .vectors (if exists) or .text otherwise.

All functions must include sizes.
Clang will do this by default for C programs, in ASM ensure you write .size after every function.
See the asm examples.
"""

# Argument parsing

objcopy = os.path.join('arm-none-eabi-objcopy')
objdump = os.path.join('arm-none-eabi-objdump')
nm = os.path.join('arm-none-eabi-nm')

(_, elf_file, stack_file) = sys.argv



RED = '\033[91m'
ORANGE = '\033[93m'
WHITE = '\033[0m'

with open(stack_file, 'w+') as logfile:
  def debugprint(*args):
    #print(*args)
    print(*args, file=logfile)
    pass

  def logprint(*args):
    #print(*args)
    print(*args, file=logfile)
    pass

  def uprint(*args, color=None):
    if color: print(color, end='')
    print(*args)
    if color: print(WHITE, end='')
    print(*args, file=logfile)


  error_stack = ""
  warning_stack = []

  """
  nm gives the symbol table which contains the start addr, size, and name of each
  symbol. Looking at `t`, or text symbols, the functions can be identified.
  $d.* sections are data sections at the end of a function, so will be excluded
  from the total length of the function.

  0000111c 00000000 t $d.1
  00001c18 00000060 t TC2_Handler
  00001544 00000036 t __muldi3

  """
  nm_out_raw = subprocess.check_output([nm, "-n", "--print-size", "--special-syms", elf_file], encoding='ascii')

  # GCC nm differs from llvm-nm here -- llvm outputs sizes for all symbols, including
  # linker symbols (i.e. ROM_LENGTH) that don't have a real size in flash/ram.
  # We need to add those back.
  nm_out = ""
  for line in nm_out_raw.splitlines():
    if len(line.split()) < 4:
      split = line.split()
      split.insert(1, "00000000")
      nm_out += " ".join(split) + "\n"
    else:
      nm_out += line + "\n"

  # [(start, end, name)]
  nm_parsed = [(int(match[0], 16), int(match[0], 16) + int(match[1], 16), match[2]) for match in re.findall(r'(?m)^([0-9a-f]+) ([0-9a-f]+) . (.*)$', nm_out)]

  # Shrink all functions to not include their extra static data section
  for (start, end, name) in nm_parsed:
    if name.startswith('$d'):
      for (s2, e2, n2) in nm_parsed:
        if s2 <= start and e2 > start:
          e2 = start

  # Parse out tags with 0 length. Because start_end_name_map is a dict,
  # linker tags can overwrite functions at the same location (i.e. _srom
  # overwriting the first function in ROM)
  start_end_name_map = { m[0]: m for m in nm_parsed if m[0] != m[1] }

  name_start_end_map = { m[2]: (m[2], m[0], m[1]) for m in nm_parsed }
  debugprint('\n'.join(str(s) for s in start_end_name_map.values()))


  """
  Get the disassembly of the text (ro executable code in flash)
   and relocate (rw initialized memory in RAM) sections.
  Some programs (ahem canprog) use functions stored in RAM,
   so disassemble the relocate section as well.

  This gives a mapping of address to function name (if present):
  `000010c0 <__fixsfsi>:`
  the stack usage of the function
  `    10c0: b5d0         	push	{r4, r6, r7, lr}`
  and the call tree of the program by finding `bl` instructions.
  """

  out = subprocess.check_output([objdump, "--disassemble", "--section=.text", "--section=.relocate", elf_file], encoding='ascii')

  debugprint(out)


  """
  Parse out all instructions for further parsing later
      1c84: b0ff xxxx    	sub	sp, #0x1fc
  """
  class Instruction:
    def __init__(self, match):
      self.addr = int(match[0], 16)
      hex_str = ''.join(reversed(match[1].split(' ')))
      self.len = len(hex_str) / 2
      self.data = int(hex_str, 16)
      self.name = match[2]
      self.arg0 = match[3]
      if self.arg0 != None:
        try:
          self.arg0 = int(self.arg0, 0)
        except: pass
      self.arg1 = match[4]
      if self.arg1 != None:
        try:
          self.arg1 = int(self.arg1, 0)
        except: pass
      self.arg2 = match[5]
      if self.arg2 != None:
        try:
          self.arg2 = int(self.arg2, 0)
        except: pass
      self.repeated = match[6] == '...'

    def __str__(self):
      return f'{hex(self.addr)} {self.addr} [{self.len}]-> {self.name} {self.arg0} {self.arg1} {self.arg2} {"*" if self.repeated else ""}'

  # See https://regex101.com/r/TZXzL7/1
  instructions = [Instruction(m) for m in re.findall(r'(?m)^ *([0-9a-f]+):\s+([0-9a-f ]*[0-9a-f]+) +\t([^\t\n]+)(?:\t(\w+|.*)(?:, \[?#?(\w+)(?:, #?([^]\n]*))?)?.*(?:\n\t\t(...))?)?', out)]

  def find_instruction(addr: int):
    return bisect.bisect_left(instructions, addr, key=operator.attrgetter('addr'))


  """
  Align an iterable of iterables for pretty printing
  """
  def align(it):
    it = [list(i) for i in it]
    if len(it) == 0: return ''

    for i, v in enumerate(it[0][:-1]):
      max_len = max(len(x[i]) for x in it) + 1
      for x in it:
        x[i] += ' ' * (max_len - len(x[i]))

    return ''.join(''.join(v) for v in it)

  """
  Extract a nested object of x.y.y.y.y.*
  into a list of x's
  """
  def unnest(obj, key):
    if obj == None: return []
    out = [obj]
    while True:
      try:
        obj = key(obj)
        if obj == None: return out
        out.append(obj)
      except AttributeError:
        return out


  class Function:
    WIP = 1
    NOTFOUND = 2
    def __init__(self, start, stack, callees):
      self.start = start
      self.end = start_end_name_map[start][1]
      self.name = start_end_name_map[start][2]
      self.stack = stack
      self.critical_path = max((functions[f] for f in callees), key=operator.attrgetter('total_stack')) if len(callees) > 0 else None
      self.total_stack = stack + (self.critical_path.total_stack if self.critical_path != None else 0)
      self.callees = callees

    def critical_path_str(self):
      return ' -> '.join(f"{f.name}({f.stack})" for f in unnest(self, operator.attrgetter("critical_path")))

    def pretty_print(self):
      return (f'{self.name}[{hex(self.start)}]:', f'{self.total_stack}', self.critical_path_str() + \
      align(((f'\n -> {functions[c].name}[{hex(c)}]', f'({functions[c].stack})', f'{functions[c].total_stack}') for c in set(self.callees))) + '\n\n')

    def __str__(self):
      callee_str = '[' + ', '.join(f'{c} ({functions[c].name} {functions[c].total_stack})' for c in self.callees)+ ']'
      return f'{hex(self.start)} {self.name}:\t\tStack {self.stack} [{self.total_stack}]\tCallees: {callee_str}'

    def __repr__(self):
      return self.__str__()

  # dictionary of start: Function|WIP
  functions = {}

  BRANCHES = ['b', 'beq', 'bne', 'bcs', 'bcc', 'bmi', 'bpl', 'bvs', 'bvc', 'bhi', 'bls', 'bge', 'blt', 'bgt', 'ble', 'bal', 'bhs', 'blo']


  """
  Every stack modification is denoted with push and pop instructions
  or add and sub instructions depending on the amount of stack space.

      1c80: b5f0         	push	{r4, r5, r6, r7, lr}
      1c82: af03         	add	r7, sp, #0xc <-- Not related to changing stack space, but is here.
      1c84: b0ff         	sub	sp, #0x1fc
      1c86: b0ff         	sub	sp, #0x1fc
      1c88: b085         	sub	sp, #0x14

  Find these and count the number of things being pushed.

  There is a possibility for one other useful format:
      1c7c: 4e67         	ldr	r6, [pc, #0x19c]        @ 0x1e1c <CAN0_Handler+0x1a4>
      1c7e: 44b5         	add	sp, r6

  In this case, the sp is a constant value, but too large to use an immediate value


  Function calls usually use `bl` instructions. Function pointers usually use
  a `blx` instruction (branch to register), figuring out where they're jumping to
  requires simulating the register values. That's too hard, just stop the call tree
  when we see it and move on.

  Other branch types (`b`, `beq`, ...) aren't used for function calls, so ignore them.

  """

  error_str = None
  blx_counter = 0

  def parse_function(start: int):
    if start in functions:
      return functions[start]
    functions[start] = Function.WIP

    global warning_stack

    # Find the index in the instructions array where this function begins
    i = find_instruction(start)
    pc = start

    # Total stack used so far and callees identified (start addresses)
    stack = 0
    callees = []

    # Find the address after the end of this function to iterate over
    (_, end_pc, name) = start_end_name_map[start]


    debugprint(f'** Parsing {name}[0x{start:08X}]')

    # See large allocation note below
    last_const_load_reg = ''
    last_const_load_value = 0

    debugprint(pc, end_pc, i, start)

    while pc < end_pc:
      inst = instructions[i]
      debugprint(inst)

      # Gap in the assembly means we're at the end of a section. It's done.
      if pc != inst.addr:
        raise RuntimeError("Gap in assembly", pc, inst.addr)

      # if not prologue_complete:
        # if not (inst.name == 'push' or inst.arg1 == 'sp' or inst.arg0 == 'sp' or (inst.name == 'ldr' and inst.arg1 == 'pc')):

      # Push is simple, each argument being pushed is 4 bytes on the stack
      if inst.name == 'push':
        stack += 4 * (len(inst.arg0.split(',')))

      # A subtraction with a constant argument is used for uninitialized block allocation
      elif inst.name == 'sub' and inst.arg0 == 'sp':
        if isinstance(inst.arg1, int):
          stack += inst.arg1
        else:
          raise RuntimeError("Unknown subtraction from stack pointer", pc, inst.addr)

      # Large allocations on the stack are sometimes written as an addition with
      # an externally-stored data value. Deal with those in two parts.
      elif inst.name == 'ldr' and inst.arg1 == 'pc':
        last_const_load_reg = inst.arg0
        last_const_load_value = inst.data

      elif inst.name == 'add' and inst.arg0 == 'sp':
        if not isinstance(inst.arg1, int):
          if inst.arg1 == last_const_load_reg:
            stack += last_const_load_value
          else:
            raise RuntimeError("Unknown addition to stack pointer", pc, inst.addr)
        else: # Constant addition, ignore it. We care about max stack usage.
          pass


      # Otherwise if this involves the stack pointer and the next thing happening is not a pop,
      # question what's happening.
      elif inst.arg0 == 'sp' and not (len(instructions) > i+1 and instructions[i+1].name == 'pop'):
        raise RuntimeError("Unknown action done to stack pointer", pc, inst.addr)

      # If it's a bl, mark the dependency and inveestigate it.
      elif inst.name == 'bl':
        inst.arg0 = int(str(inst.arg0), 16) # Convert the branch address from hex string to integer
        debugprint(functions, inst.arg0, inst.arg0 not in functions)
        if inst.arg0 in start_end_name_map:
          callees.append(inst.arg0)

          # If we have not parsed it yet, parse it
          if inst.arg0 not in functions:
            parse_function(inst.arg0)

          # If it is marked WIP, then we have a recursive dependency.
          if functions[inst.arg0] == Function.WIP:
            raise RuntimeError(f"Recursion detected between {name} and {start_end_name_map[inst.arg0][2]}. I'm not happy.", pc, inst.addr)
        else:
          try:
            inst_arg0_int = int(inst.arg0, base=16)
          except TypeError:
            inst_arg0_int = inst.arg0

          is_local_bl = inst_arg0_int > start and inst_arg0_int <= end_pc
          if not is_local_bl:
            debugprint(f'** Warning: Could not find target for bl to {inst.arg0}')
            warning_stack += [f'** Warning: Could not find target for bl to {inst.arg0} **']
          else:
            debugprint(f'** Found local bl to {inst.arg0}')


      # BLX is an arbitrary-destination branch and link. Too hard to figure out.
      elif inst.name == 'blx':
        global blx_counter
        blx_counter += 1
        # Keep the terminal warning spam to a minimum
        debugprint(f" ** Warning: Programs using blx instruction are not supported (at {pc:.0f}) ")
        if blx_counter < 3:
          warning_stack += [f"** Warning: Programs using blx instruction are not supported (at {pc:.0f})"]
        elif blx_counter == 3:
          warning_stack += [f"** Warning: Programs using blx instruction are not supported (multiple, ignoring future blx)"]

      # If this instruction is repeating, repeat until the pc matches the next
      # different instruction
      pc += inst.len

      if inst.repeated:
        if len(instructions) > i+1 and instructions[i+1].addr != pc:
          continue

      i += 1

    functions[start] = Function(start, stack, callees)
    return functions[start]


  """
  Parse the exception table (47 words) to find the interrupt handler entrypoints.
  If a handler address is 0, it is unused.

  The first word is the stack pointer, and
  the next 46 are handlers.

  Important handlers are Reset -> Start of code on boot.
  Any other handler -> Can interrupt Reset
  Hardfault -> Can interrupt any other handler.

  The total depth of Reset + max(others) + Hardfault is the max depth

    void* stack_ptr;

    void* pfnReset_Handler;
    void* pfnNonMaskableInt_Handler;
    void* pfnHardFault_Handler;
    void* pvReservedM12;
    void* pvReservedM11;

    void* pvReservedM10;
    void* pvReservedM9;
    void* pvReservedM8;
    void* pvReservedM7;
    void* pvReservedM6;

    void* pfnSVCall_Handler;
    void* pvReservedM4;
    void* pvReservedM3;
    void* pfnPendSV_Handler;
    void* pfnSysTick_Handler;

    /* Peripheral handlers */
    void* pfnSYSTEM_Handler;                /*  0 Main Clock, 32k Oscillators Control, Oscillators Control, Peripheral Access Controller, Power Manager, Supply Controller, Trigger Allocator */
    void* pfnWDT_Handler;                   /*  1 Watchdog Timer */
    void* pfnRTC_Handler;                   /*  2 Real-Time Counter */
    void* pfnEIC_Handler;                   /*  3 External Interrupt Controller */
    void* pfnFREQM_Handler;                 /*  4 Frequency Meter */
    void* pfnTSENS_Handler;                 /*  5 Temperature Sensor */
    void* pfnNVMCTRL_Handler;               /*  6 Non-Volatile Memory Controller */
    void* pfnDMAC_Handler;                  /*  7 Direct Memory Access Controller */
    void* pfnEVSYS_Handler;                 /*  8 Event System Interface */
    void* pfnSERCOM0_Handler;               /*  9 Serial Communication Interface 0 */
    void* pfnSERCOM1_Handler;               /* 10 Serial Communication Interface 1 */
    void* pfnSERCOM2_Handler;               /* 11 Serial Communication Interface 2 */
    void* pfnSERCOM3_Handler;               /* 12 Serial Communication Interface 3 */
    void* pfnSERCOM4_Handler;               /* 13 Serial Communication Interface 4 */
    void* pfnSERCOM5_Handler;               /* 14 Serial Communication Interface 5 */
    void* pfnCAN0_Handler;                  /* 15 Control Area Network 0 */
    void* pfnCAN1_Handler;                  /* 16 Control Area Network 1 */
    void* pfnTCC0_Handler;                  /* 17 Timer Counter Control 0 */
    void* pfnTCC1_Handler;                  /* 18 Timer Counter Control 1 */
    void* pfnTCC2_Handler;                  /* 19 Timer Counter Control 2 */
    void* pfnTC0_Handler;                   /* 20 Basic Timer Counter 0 */
    void* pfnTC1_Handler;                   /* 21 Basic Timer Counter 1 */
    void* pfnTC2_Handler;                   /* 22 Basic Timer Counter 2 */
    void* pfnTC3_Handler;                   /* 23 Basic Timer Counter 3 */
    void* pfnTC4_Handler;                   /* 24 Basic Timer Counter 4 */
    void* pfnADC0_Handler;                  /* 25 Analog Digital Converter 0 */
    void* pfnADC1_Handler;                  /* 26 Analog Digital Converter 1 */
    void* pfnAC_Handler;                    /* 27 Analog Comparators */
    void* pfnDAC_Handler;                   /* 28 Digital Analog Converter */
    void* pfnSDADC_Handler;                 /* 29 Sigma-Delta Analog Digital Converter */
    void* pfnPTC_Handler;                   /* 30 Peripheral Touch Controller */
  """
  UINT32 = next(code for code in array.typecodes if code.isupper() and array.array(code).itemsize == 4)

  # GCC objcopy differs from llvm-objcopy here -- llvm-objcopy can dump the output
  # to shell, GCC can't. Just make a temporary file and read it back.
  bin_file = elf_file.replace(".elf", ".bin")
  subprocess.check_output([objcopy, '-O', 'binary', '--only-section=.vectors', elf_file, bin_file])
  out = bytes()
  with open(bin_file, 'br') as bin:
    out = bin.read()
  os.remove(bin_file) # Clean up

  if len(out) == 0:
    out = subprocess.check_output([objcopy, '-O', 'binary', elf_file, '-'])
  if len(out) > 47 * 4:
    out = out[:47*4]

  if 'exception_table' in name_start_end_map:
    except_len = name_start_end_map['exception_table'][2] - name_start_end_map['exception_table'][1]
    if len(out) > except_len:
      out = out[:except_len]

  debugprint('Exception Table Raw: ', out)

  exception_table = [x & 0xFFFFFFE for x in array.array(UINT32, out)]

  debugprint('Exception Table: [' + ', '.join(f'0x{e:08X}' for e in exception_table) + ']')

  """
  Parse the functions starting at the exception entrypoints given by the exception table
  """

  if len(exception_table) == 0:
    used_stack = 0
    error_stack = f' ** Error: Could not find exception table **'
  else:
    vectors = set(x for x in exception_table[1:] if x != 0)

    debugprint('\n>> PARSING FUNCTIONS <<\n')
    for v in vectors:
      try:
        parse_function(v)
      except RuntimeError as x:
        debugprint(x)
        error_stack = f'** Error: {x.args[0]} **'
      except KeyError as x:
        debugprint(x)
        error_stack = f'** Error: Could not find function at address 0x{x.args[0]:08X} **'

    if not error_stack:
      logprint('\n>> STACK ANALYSIS RESULTS <<\n')
      logprint(align(f.pretty_print() for f in functions.values()))

      # Find Reset, Hardfault, and maxmimum other exception stack for those which exist. Reset always exists or error.
      other_exceptions = ([exception_table[2]] + (exception_table[4:] if len(exception_table) > 4 else []) if len(exception_table) > 2 else [])

      reset_vector = functions[exception_table[1]]
      hardfault_vector = functions[exception_table[3]] if len(exception_table) > 3 and exception_table[3] != 0 else None
      max_vector = max((functions[x] for x in other_exceptions if x != 0), key=lambda x: x.total_stack) if len(other_exceptions) > 0 else None

      INTERRUPT_STACK = 32
      used_stack = reset_vector.total_stack
      critical_path_str = '    -> ' + reset_vector.critical_path_str()
      if max_vector != None:
        used_stack += INTERRUPT_STACK + max_vector.total_stack
        critical_path_str += '\n    -> Interrupt(32) + ' + max_vector.critical_path_str()
      if hardfault_vector != None:
        used_stack += INTERRUPT_STACK + hardfault_vector.total_stack
        critical_path_str += '\n    -> Interrupt(32) + ' + hardfault_vector.critical_path_str()




  """
  Print resource summary usage for flash, sram, and stack.
  """

  uprint(f'-- Resource Usage Summary for {elf_file} --')
  error_flash = ""
  error_sram = ""

  try:
    total_flash = name_start_end_map['ROM_LENGTH'][1]
    used_flash = name_start_end_map['_erom'][1] - name_start_end_map['_srom'][1]
  except KeyError as e:
    total_flash = 0
    used_flash = 0
    error_flash = f'** Could not find required linker symbol {e.args[0]} **'

  try:
    total_sram = name_start_end_map['RAM_LENGTH'][1]
    used_sram = name_start_end_map['_eram'][1] - name_start_end_map['_sram'][1]
  except KeyError as e:
    total_sram = 0
    used_sram = 0
    error_sram = f'** Could not find required linker symbol {e.args[0]} **'

  try:
    total_stack = name_start_end_map['_estack'][1] - name_start_end_map['_sstack'][1]
  except KeyError as e:
    total_stack = 0
    error_stack = f'** Could not find required linker symbol {e.args[0]} **'

  used_sram -= total_stack
  total_sram -= total_stack



  if not error_flash:
    usage_flash = (1 if used_flash == 0 else float('inf')) if total_flash == 0 else used_flash / total_flash
    color_flash = WHITE if usage_flash < 0.8 else ORANGE if usage_flash <= 1 else RED
    if color_flash == RED: error_flash = '** Stack usage out of bounds **'
    uprint(f'  FLASH: {usage_flash:0.2%} ({used_flash} / {total_flash})', color=color_flash)
  else:
    uprint(f'  FLASH: ??? / ??? {error_flash}', color=RED)

  if not error_sram:
    usage_sram = (1 if used_sram == 0 else float('inf')) if total_sram == 0 else used_sram / total_sram
    color_sram = WHITE if usage_sram < 0.8 else ORANGE if usage_sram <= 1 else RED
    if color_sram == RED: error_sram = '** Stack usage out of bounds **'
    uprint(f'  SRAM:  {usage_sram:0.2%} ({used_sram} / {total_sram})', color=color_sram)
  else:
    uprint(f'  SRAM: ??? / ??? {error_sram}', color=RED)

  if not error_stack:
    usage_stack = (1 if used_stack == 0 else float('inf')) if total_stack == 0 else used_stack / total_stack
    color_stack = WHITE if usage_stack < 0.5 else ORANGE if usage_stack <= 1 else RED
    if color_stack == RED: error_stack = '** Stack usage out of bounds **'
    uprint(f'  STACK: {usage_stack:0.2%} ({used_stack} / {total_stack})', color=color_stack)
    if warning_stack:
      uprint('\n'.join('    ' + i for i in warning_stack), color=ORANGE)
    uprint(critical_path_str)
  else:
    uprint(f'  STACK: ??? / ??? {error_stack}', color=RED)

  exit(bool(error_flash or error_sram or error_stack))