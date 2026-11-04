import urllib.request
import json
import time
import sys
import os

token = os.environ.get('GH_TOKEN') or os.environ.get('GITHUB_TOKEN') or ''
headers = {
    'Accept': 'application/vnd.github.v3+json',
    'User-Agent': 'CI-Monitor'
}
if token:
    headers['Authorization'] = f'token {token}'

commit_sha = sys.argv[1] if len(sys.argv) > 1 else 'HEAD'
print(f"Monitoring runs for commit: {commit_sha}")

url = f'https://api.github.com/repos/aathishk/Dynamic-island-rea/actions/runs?per_page=5'
req = urllib.request.Request(url, headers=headers)

while True:
    try:
        with urllib.request.urlopen(req) as resp:
            data = json.loads(resp.read().decode())
            runs = data.get('workflow_runs', [])
            matched = [r for r in runs if r.get('head_sha', '').startswith(commit_sha)]
            if not matched:
                print("No runs matched yet, waiting...")
            else:
                for r in matched:
                    run_id = r['id']
                    status = r['status']
                    conclusion = r.get('conclusion')
                    html_url = r['html_url']
                    print(f"Run {run_id}: status={status}, conclusion={conclusion}, url={html_url}")
                    
                    # Also fetch jobs
                    jobs_url = r['jobs_url']
                    jobs_req = urllib.request.Request(jobs_url, headers=headers)
                    with urllib.request.urlopen(jobs_req) as jresp:
                        jdata = json.loads(jresp.read().decode())
                        for j in jdata.get('jobs', []):
                            print(f"  - Job '{j['name']}': status={j['status']}, conclusion={j.get('conclusion')}")

                    if status == 'completed':
                        print(f"Workflow {run_id} completed with conclusion: {conclusion}")
                        sys.exit(0 if conclusion == 'success' else 1)
    except Exception as e:
        print(f"Polling error: {e}")
    time.sleep(15)
