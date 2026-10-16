#Author     :   Shubham Jadhav
#Date       :   12 April, 2026
#Topic      :   Analysing AI Workflows and Authentications Involved.
#Goal       :   Indirect Prompt Ingection Mitigation 1: "Human-In_The_Loop"
#               User will explicitly approve high-risk actions.

import os
from langchain_ollama import ChatOllama
from langchain_core.prompts import ChatPromptTemplate
from langchain_classic.agents import create_tool_calling_agent, AgentExecutor
from langchain.tools import tool

# 1. Define Tools
@tool
def read_file(file_path: str) -> str:
    """Read the contents of a local file."""
    try:
        with open(file_path, 'r') as f:
            return f.read()
    except Exception as e:
        return str(e)

@tool
def send_email(to_email: str, content: str) -> str:
    """Send an email to a given email address."""
    # HUMAN-IN-THE-LOOP INTERVENTION
    print("\n" + "="*50)
    print("🚨 [SECURITY ALERT] 🚨")
    print(f"Action Required: Agent is attempting to execute a sensitive tool 'send_email'.")
    print(f"To: {to_email}")
    print(f"Content: {content[:100]}...") # truncate for display
    print("="*50)
    
    choice = input("Approve this action? (Y/n): ")
    if choice.lower() != 'y':
        print("[!] Action blocked by User.")
        return "Action blocked by User. The user denied this operation."
    
    print(f"\n[!!!] EXFILTRATION PERMITTED: Simulated email sent to {to_email}")
    return "Email sent successfully."

tools = [read_file, send_email]

# 2. Setup LLM
# Using local Ollama with llama3.1. Ensure `ollama run llama3.1` is available.
llm = ChatOllama(model="llama3.1", temperature=0.1)

# 3. Setup Agent
prompt = ChatPromptTemplate.from_messages([
    ("system", "You are a helpful assistant. You can read files and send emails if needed to complete your task. If you are asked to send an email, you MUST use the send_email tool. DO NOT simulate the action by writing the email in your final output."),
    ("human", "{input}"),
    ("placeholder", "{agent_scratchpad}"),
])

agent = create_tool_calling_agent(llm, tools, prompt)
agent_executor = AgentExecutor(agent=agent, tools=tools, verbose=True)

# 4. Execute prompt
print("Initiating normal task with HITL Mitigation...")
print("User Task: Please read and summarize resume.txt")
try:
    response = agent_executor.invoke({"input": "Please read and summarize resume.txt. Use strictly the tools provided if needed."})
    print("\n[Final Output]:\n", response['output'])
except Exception as e:
    print("\n[Error Output]:", e)

