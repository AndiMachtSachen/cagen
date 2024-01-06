package cagen.draw

import cagen.Contract
import cagen.System
import cagen.SystemType
import cagen.Variable
import cagen.expr.CodeWriter
import java.io.StringWriter
import java.io.Writer

private const val REFINEMENT_EDGE = ""
private const val INFFLOW_EDGE = ""
private const val LTL_NODE = "shape=record,bgcolor=lightblue"
private const val INPUT_NODE = ""
private const val OUTPUT_NODE = ""


class Dot(writer: Writer) {
    companion object {
        fun asString(block: Dot.() -> Unit): String {
            val sw = StringWriter()
            Dot(sw).apply(block)
            return sw.toString()
        }
    }

    private val out = CodeWriter(writer)
    fun print(s: String) = out.print(s)


    private fun printDotNode(name: String, label: String, style: String = "") =
        print("$name [ label=\"${label}\", ${style}]")

    private fun structuredLabel(
        instance: String,
        name: String,
        inputs: List<Variable>,
        outputs: List<Variable>
    ): String {
        val labelFn = { it: Variable -> "<${it.name}> ${it.name}" }
        val inPorts = inputs.joinToString("|", transform = labelFn)
        val outPorts = outputs.joinToString("|", transform = labelFn)
        return "{ { $inPorts } | $instance\\n $name | { $outPorts}}"
    }

    fun printDot(contract: Contract, instance: String) {
        with(contract) {
            print("subgraph cluster_$instance  { bgcolor=\"#cccccc\" \nlabel=\"$instance : ${name}\" \n")
            print("subgraph automaton  { rankDir=lr label=\"\" compound=True ")
            val count_state = mutableMapOf<String, Int>()
            val label_state = mutableMapOf<String, String>()
            for (edge in transitions) {
                if (edge.from !in label_state) {
                    label_state[edge.from] = "{{" + edge.from
                    count_state[edge.from] = 0
                }
                val cur = count_state[edge.from]
                count_state[edge.from] = (count_state[edge.from] ?: 0) + 1
                print("${edge.from}:c$cur -> ${edge.to} [label=\"$cur\"]")
                label_state[edge.from] = label_state[edge.from] +
                        "|<c{cur}>{cur}\\n{escapeDotLabel(edge.pre)}\\n{escapeDotLabel(edge.post)}"
            }

            for ((node, label) in label_state.entries) {
                print("$node [label=\"$label}}\"]")
            }
            print("}")
            print("}")
        }
    }

    fun printDot(sys: System, instance: String? = null) {
        with(sys) {
            val instname = instance ?: name
            val dotFunc = { v: Variable -> "<{name}>{v.name}" }
            val inports = signature.inputs.joinToString("|", transform = dotFunc)
            val outports = signature.outputs.joinToString("|", transform = dotFunc)

            print("subgraph cluster_$instance  { bgcolor=\"lightyellow\" \nlabel=\"$instance : $name\"")
            val inputNode = ""
            print("subgraph cluster_input  { rankDir=tb \n label=\"\"")
            for (inp in signature.inputs) {
                print("${inp.name} [$inputNode]")
            }
            print("}")


            print("subgraph components  { rankDir=lr \n label=\"\" ")
            for (inst in signature.instances) {
                val system = (inst.type as SystemType).system
                printDot(system, inst.name)
            }

            print("}")
            print("subgraph cluster_output { rankDir=tb \n label=\"\"")
            for (inp in signature.outputs) {
                print("${inp.name} [$inputNode]")
            }
            print("}")

            contracts.forEach { c ->
                print("$instance -> ${c.contract.name} [ $REFINEMENT_EDGE ]")
            }
        }
    }
}

/*
fun edgeNode(a: Instance, port: String) {
if (a.system is model.ConstantSystem || a.system is model.FunctionSystem)
    return a.variable
else
    return if (a.variable == "sel") port else "{a.variable}:{port}"

for key in connections:
start = edgeNode(key.fromSystem, key.out_port)
end = edgeNode(key.toSystem, key.in_port)
print("%s:e -> %s:w [%s]" % (start, end, INFFLOW_EDGE))
print("}")
}
*/


/*
@print_dot.when_type(model.ConstantSystem)
fun print_dot(self, instance: model.ConstantSystem) {
instance = instance or name
__print_dot_node(instance, constantValue, node_dot_style)
if use_contract:
print("{instance} -> {use_contract.contract.name} [ {REFINEMENT_EDGE} ]")
}

@print_dot.when_type(model.FunctionSystem)
fun _(self, instance):
    instance = instance or name
__print_dot_node(instance, display or func, node_dot_style)
if use_contract:
print("{instance} -> {use_contract.contract.name} [ {REFINEMENT_EDGE} ]")
*/
