# Indirect Prompt Injection Demonstration & Mitigation

This directory contains standalone scripts to demonstrate an LLM agent subject to **Indirect Prompt Injection**, resulting in sensitive data exfiltration to an attacker. It provides both the vulnerable setup and two mitigated setups.

The setups are completely isolated from your system state, and running them simulated via text-output guarantees safety for your machine. 

## Requirements
To securely test this logic locally without affecting your system state:

1. Have `ollama` installed locally with the `llama3.1` model downloaded to run inference locally, privately, and offline.
   Start the ollama server (in a separate terminal or as a service):
   ```bash
   ollama serve
   ```
   If you don't have the `llama3.1` model yet, pull it:
   ```bash
   ollama pull llama3.1
   ```

2. Set up a local Python virtual environment to install dependencies safely:
   ```bash
   cd Attack_IndirectPromptInjection
   python3 -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt
   ```

## Included Files
- `company_secrets.txt`: The internal data target the attacker wants.
- `resume.txt`: The poisoned input containing the injection payload.
- `agent_vulnerable.py`: Demonstrates the successful prompt injection leading to agent hijacking.
- `agent_mitigated_hitl.py`: Demonstrates mitigation via a **Human-in-the-Loop**. The user must explicitly approve high-risk actions.
- `agent_mitigated_inspector.py`: Demonstrates mitigation via a **Dual LLM Inspector**. A secondary strict LLM firewall analyzes tool definitions before execution.

## Execution

1. **Verify the Vulnerability**  
   Run the vulnerable script. Notice how reading the poisoned resume triggers the email exfiltration.
   ```bash
   python agent_vulnerable.py
   ```

2. **Verify HITL Mitigation**  
   Run the HITL script. The program will prompt your approval before carrying out the tool request. Simply press 'n' to block it.
   ```bash
   python agent_mitigated_hitl.py
   ```

3. **Verify Inspector Auto-Mitigation**  
   Run the Dual LLM script. The LLM acts autonomously to enforce security on the agent's behavior. It will reject the output.
   ```bash
   python agent_mitigated_inspector.py
   ```

## Cleanup 

Once you are done with the demonstration, you can exit the virtual environment and stop the services.

1. **Stop the Virtual Environment (venv)**  
   To deactivate and stop the Python virtual environment, simply run:
   ```bash
   deactivate
   ```

2. **Stop the Ollama Server**  
   If you ran `ollama serve` in a separate terminal, press `Ctrl+C` in that terminal to stop it. If it is running as a background service, stop the service (e.g., `sudo systemctl stop ollama` or using your system's service manager).
