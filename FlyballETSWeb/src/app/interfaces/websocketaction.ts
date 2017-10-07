export interface WebsocketAction {
   action: string,
   response: WebsocketActionResult
}

export interface WebsocketActionResult {
   success: boolean,
   error: string
}
