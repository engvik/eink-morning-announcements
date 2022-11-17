<script>
    import {onMount} from 'svelte';

    let message = 'Loading ...';
    let update = 'N/A';
    let error = '';

    async function getCurrentMessage() {
        const res = await fetch(import.meta.env.VITE_BACKEND_BASE_URL + "/api/message");
        
        if (res.status != 200) {
            if (res.status == 404) {
                message = '';
                return;
            }

            const text = await res.text();
            error = 'Error fetching message: ' + text;
            message = '';
            return;
        }

        const json = await res.json();
        message = json.message;
        update = new Date(json.time).toString();
    }

    async function updateMessage() {
        const res = await fetch(import.meta.env.VITE_BACKEND_BASE_URL + "/api/message", {
            method: 'POST',
            body: JSON.stringify({message})
        });

        if (res.status != 200) {
            const text = await res.text();
            error = 'Error updating message: ' + text;
            message = '';
            return;
        }

        update = new Date().toString();
    }

    function clearMessage() {
        message = '';
    }

    onMount(async () => {
        await getCurrentMessage();
    });

</script>

<div id="input-form">
    <textarea bind:value={message}></textarea>
    <button type="button" on:click={updateMessage}>Update</button>
    <button type="button" on:click={clearMessage}>Clear</button>
    <span>Last update: {update}</span>
    <span>{error != '' ? 'Error: ' + error : ''}</span>
</div>

<style>
    textarea {
        width: 60%;
        height: 30vh;
    }

    #input-form {
        display: flex;
        flex-direction: column;
    }

    button {
        margin: 5px 0px;
        width: 20%;
    }
</style>
