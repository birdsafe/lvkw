# Documentation Standards

LVKW maintains a strict separation between **User-Facing Documentation** (what the library does) and **Internal Documentation** (how it does it). 

## User-Facing Documentation
*   **Locations:** `include/lvkw/` (Doxygen headers), `docs/user_guide/`, `README.md`. `examples/`
*   **Focus:** Semantics, Contracts, and Observable Behavior.
*   **Rule:** Document the **What**, not the **How**.

Users need to understand the guarantees and constraints of the API, but they should not be exposed to internal implementation details that is not relevant to them.

**Example: The Event Queue**
- **Do document:** That it is a queue, that it has a capacity, the eviction/compression strategy (semantics), and the thread-affinity rules.
- **Don't document:** That it is implemented as a lock-free ring buffer. The "ring buffer" aspect is an internal optimization choice; as long as it behaves like a queue with the promised semantics, the user doesn't need to know the underlying data structure.

### Why this matters:
1.  **Encapsulation:** It prevents users from making assumptions based on internal details that aren't part of the public contract.
2.  **Maintainability:** We can swap a ring buffer for a linked list of pages tomorrow without having to update the User Guide.
3.  **Clarity:** It reduces the cognitive load for users who just want to know how to use the library correctly.

## Internal Documentation
*   **Locations:** `docs/dev_guide/`, `src/` (inline comments)
*   **Focus:** Architecture, Data Structures, and "The Why".
*   **Rule:** Document the **How** and the **Trade-offs**.

This is where we discuss things like cache-line alignment, specific hardware-backend quirks, and choice of algorithms.

## Tone and Language
*   **Be Objective:** Focus on technical capabilities rather than marketing.
*   **Conversational is fine:** No need to be excessively dry and formal.
*   **Avoid Adjectives:** Do not use words like "powerful", "fast", "flexible", or "pleasant" in technical documentation. If a feature is fast, the technical metrics or design choices should speak for themselves.
*   **No Self-Congratulation:** Avoid language that praises the library or its implementation. The documentation's purpose is to inform, not to sell. 
*   **No Putting-down of the competition:** Avoid focusing on what other libraries might be doing differently. This project can stand on its own.

### Exceptions
A few targeted exceptions to the tone rules exist in `README.md` and the `user_guide` index. Notably:

- The foreword of the README.md does need a positive tone. Come on now...
- We drive new users away from the user_guide, which is meant to be deep-dives, and instead towards the headers, which
  are specifically designed to serve as an efficient reference manual in of themselves. We need to lay it on thick as far as on the headers' readability is concerned.